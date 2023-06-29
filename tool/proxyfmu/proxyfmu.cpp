
#include "handlers/boot_service_handler.hpp"
#include "handlers/fmu_service_handler.hpp"
#include "rng.hpp"

#include "ecos/lib_info.hpp"

#include <CLI/CLI.hpp>
#include <functional>
#include <iostream>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <utility>

using namespace proxyfmu::thrift;
using namespace proxyfmu::server;

using namespace ::apache::thrift;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

namespace
{

const int port_range_min = 49152;
const int port_range_max = 65535;

const int max_port_retries = 10;

const int SUCCESS = 0;
const int COMMANDLINE_ERROR = 1;
const int UNHANDLED_ERROR = 2;


class ServerReadyEventHandler : public TServerEventHandler
{

private:
    std::function<void()> callback_;

public:
    explicit ServerReadyEventHandler(std::function<void()> callback)
        : callback_(std::move(callback))
    { }

    void preServe() override
    {
        callback_();
    }
};

void wait_for_input()
{
    std::cout << '\n'
              << "Press any key to quit...\n";
    // clang-format off
    while (std::cin.get() != '\n');
    //clang-format on
    std::cout << "Done." << std::endl;
}

int run_application(const std::string& fmu, const std::string& instanceName)
{
    std::unique_ptr<TSimpleServer> server;
    auto stop = [&]() {
        server->stop();
    };
    std::shared_ptr<fmu_service_handler> handler(new fmu_service_handler(fmu, instanceName, stop));
    std::shared_ptr<TProcessor> processor(new FmuServiceProcessor(handler));

    std::shared_ptr<TTransportFactory> transportFactory(new TFramedTransportFactory());
    std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    rng rng(port_range_min, port_range_max);

    int port;
    int final_port = -1;
    for (auto i = 0; i < max_port_retries; i++) {
        port = rng.next();

        std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
        server = std::make_unique<TSimpleServer>(processor, serverTransport, transportFactory, protocolFactory);
        server->setServerEventHandler(std::make_shared<ServerReadyEventHandler>([port, &final_port] {
            final_port = port;
            std::cout << "[proxyfmu] port=" << std::to_string(final_port) << std::endl;
        }));

        try {

            server->serve();
            break;

        } catch (const TTransportException& ex) {
            std::cout << "[proxyfmu] " << ex.what()
                      << ". Failed to bind to port " << std::to_string(port)
                      << ". Retrying with another one. Attempt " << std::to_string(i + 1)
                      << " of " << std::to_string(max_port_retries) << ".." << std::endl;
        }
    }

    if (final_port != -1) {
        return SUCCESS;
    } else {
        std::cerr << "[proxyfmu] Unable to bind after max number of retries.." << std::endl;
        return UNHANDLED_ERROR;
    }
}

int run_boot_application(const int port)
{
    std::unique_ptr<TSimpleServer> server;
    std::shared_ptr<boot_service_handler> handler(new boot_service_handler());
    std::shared_ptr<TProcessor> processor(new BootServiceProcessor(handler));

    std::shared_ptr<TTransportFactory> transportFactory(new TFramedTransportFactory());
    std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    server = std::make_unique<TSimpleServer>(processor, serverTransport, transportFactory, protocolFactory);

    std::thread t([&server] { server->serve(); });

    wait_for_input();

    server->stop();
    t.join();

    return SUCCESS;
}

int printHelp(CLI::App& desc)
{
    std::cout << desc.help() << std::endl;
    return SUCCESS;
}

std::string version()
{
    const auto v = ecos::library_version();
    std::stringstream ss;
    ss << "v" << v.major << "." << v.minor << "." << v.patch;
    return ss.str();
}

} // namespace

int main(int argc, char** argv)
{

    CLI::App app{"proxyfmu"};

    app.set_version_flag("-v,--version", version());
    app.add_option("--fmu", "Location of the fmu to load.");
    app.add_option("--instanceName", "Name of the slave instance.");

    CLI::App* sub = app.add_subcommand("boot");
    sub->add_option("--port", "Specify the network port to be used.")->required();

    if (argc == 1) {
        return printHelp(app);
    }

    try {

        CLI11_PARSE(app, argc, argv);

        if (*sub) {

            const auto port = sub->get_option("--port")->as<int>();
            return run_boot_application(port);

        } else {
            const auto fmu = app["--fmu"]->as<std::string>();
            const auto fmuPath = std::filesystem::path(fmu);
            if (!std::filesystem::exists(fmuPath)) {
                std::cerr << "[proxyfmu] No such file: '" << std::filesystem::absolute(fmuPath) << "'";
                return COMMANDLINE_ERROR;
            }

            const auto instanceName = app["--instanceName"]->as<std::string>();

            return run_application(fmu, instanceName);
        }

    } catch (const std::exception& e) {
        std::cerr << "[proxyfmu] Unhandled Exception reached the top of main: " << e.what() << ", application will now exit" << std::endl;
        return UNHANDLED_ERROR;
    }
}
