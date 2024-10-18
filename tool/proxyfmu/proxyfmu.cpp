
#include "handlers/boot_service_handler.hpp"

#include "ecos/lib_info.hpp"

#include "fmilibcpp/fmu.hpp"
#include "fmilibcpp/slave.hpp"
#include "proxyfmu/functors.hpp"
#include "simple_socket/TCPSocket.hpp"
#include "simple_socket/util/byte_conversion.hpp"
#include "simple_socket/util/port_query.hpp"
#include <CLI/CLI.hpp>
#include <msgpack.hpp>

#include <functional>
#include <iostream>
#include <random>
#include <utility>

using namespace proxyfmu::server;


namespace
{

const int port_range_min = 49152;
const int port_range_max = 65535;

const int SUCCESS = 0;
const int COMMANDLINE_ERROR = 1;
const int UNHANDLED_ERROR = 2;


void wait_for_input()
{
    std::cout << '\n'
              << "Press any key to quit...\n";
    // clang-format off
    while (std::cin.get() != '\n');
    //clang-format on
    std::cout << "Done." << std::endl;
}

void sendStatus(simple_socket::SimpleConnection& conn, bool status)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, status);
    conn.write(sbuf.data(), sbuf.size());
}

template <typename T>
void sendStatusAndValues(simple_socket::SimpleConnection& conn, bool status, const std::vector<T>& values)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, status);
    msgpack::pack(sbuf, values);
    conn.write(sbuf.data(), sbuf.size());
}

void client_handler(std::unique_ptr<simple_socket::SimpleConnection> conn, fmilibcpp::slave* slave)
{

    std::vector<uint8_t> buffer(1024*64);
    while (true) {
        const int read = conn->read(buffer.data(), buffer.size());

    int func;
    std::size_t offset = 0;
    auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
    oh.get().convert(func);

    const auto f = ecos::int_to_enum(func);
    switch (f) {
        case ecos::functors::setup_experiment: {

            double startTime;
            double endTime;
            double tolerance;
            oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
            oh.get().convert(startTime);
            oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
            oh.get().convert(endTime);
            oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
            oh.get().convert(tolerance);
            const auto status = slave->setup_experiment(startTime, endTime, tolerance);
            sendStatus(*conn, status);

        }
        break;
        case ecos::functors::enter_initialization_mode: {
            const auto status = slave->enter_initialization_mode();
            sendStatus(*conn, status);
        }
        break;
        case ecos::functors::exit_initialization_mode: {
            const auto status = slave->exit_initialization_mode();
            sendStatus(*conn, status);
        }
        break;
        case ecos::functors::step: {
            double currentTime;
            double stepSize;
            oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
            oh.get().convert(currentTime);
            oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
            oh.get().convert(stepSize);
            const auto status = slave->step(currentTime, stepSize);
            sendStatus(*conn, status);
        }
        break;
        case ecos::functors::terminate: {
            const auto status = slave->terminate();
            sendStatus(*conn, status);
        }
        break;
        case ecos::functors::freeInstance: {
            slave->freeInstance();
            return;
        }
        break;
        case ecos::functors::read_int: {
            std::vector<fmilibcpp::value_ref> vr;

            oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
            oh.get().convert(vr);

            std::vector<int> values(vr.size());
            const auto status = slave->get_integer(vr, values);

           sendStatusAndValues(*conn, status, values);
        }
        break;
        case ecos::functors::read_real: {
            std::vector<fmilibcpp::value_ref> vr;

            oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
            oh.get().convert(vr);

            std::vector<double> values(vr.size());
            const auto status = slave->get_real(vr, values);

            sendStatusAndValues(*conn, status, values);
        }
        break;
        case ecos::functors::read_string: {
            std::vector<fmilibcpp::value_ref> vr;

            oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
            oh.get().convert(vr);

            std::vector<std::string> values(vr.size());
            const auto status = slave->get_string(vr, values);

            sendStatusAndValues(*conn, status, values);
        }
        break;
        case ecos::functors::read_bool: {
            std::vector<fmilibcpp::value_ref> vr;

            oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), read, offset);
            oh.get().convert(vr);

            std::vector<bool> values(vr.size());
            const auto status = slave->get_boolean(vr, values);

            sendStatusAndValues(*conn, status, values);
        }
        break;
    }
    }

}

int run_application(const std::string& fmu, const std::string& instanceName)
{

    const auto port = simple_socket::getAvailablePort(port_range_min, port_range_max);
    if (!port) {
        std::cerr << "[proxyfmu] Unable to locate free port number.." << std::endl;
        return UNHANDLED_ERROR;
    }

    auto model = fmilibcpp::loadFmu(fmu);
    auto instance = model->new_instance(instanceName);

    simple_socket::TCPServer server(*port);
    auto con = server.accept();
    client_handler(std::move(con), instance.get());

    return SUCCESS;
}

int run_boot_application(const int port)
{

    boot_service_handler handler;
    simple_socket::TCPServer server(port);

    std::thread server_thread([&] {
        auto conn = server.accept();

        std::vector<uint8_t> buffer(4);
        conn->readExact(buffer);

        const auto msgSize = simple_socket::decode_uint32(buffer);
        buffer.clear();
        buffer.resize(msgSize);
        conn->readExact(buffer);

        std::string fmuName;
        std::string instanceName;
        std::string data;

        std::size_t offset = 0;
        auto oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), msgSize, offset);
        oh.get().convert(fmuName);
        oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), msgSize, offset);
        oh.get().convert(instanceName);
        oh = msgpack::unpack(reinterpret_cast<const char*>(buffer.data()), msgSize, offset);
        oh.get().convert(data);

        int16_t instance_port = handler.loadFromBinaryData(fmuName, instanceName, data);
        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, instance_port);
        conn->write(sbuf.data(), sbuf.size());

    });

    wait_for_input();

    server.close();
    server_thread.join();

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
            if (!exists(fmuPath)) {
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
