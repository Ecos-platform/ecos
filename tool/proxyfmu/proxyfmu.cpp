
#include "boot_service_handler.hpp"
#include "client_handler.hpp"

#include "ecos/lib_info.hpp"
#include "ecos/util/uuid.hpp"

#include "simple_socket/TCPSocket.hpp"
#include "simple_socket/UnixDomainSocket.hpp"
#include "simple_socket/util/byte_conversion.hpp"
#include "simple_socket/util/port_query.hpp"
#include <flatbuffers/flexbuffers.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cli11/CLI11.h>
#include <iostream>
#include <random>
#include <utility>

using namespace ecos;
using namespace ecos::proxy;


namespace
{

const int port_range_min = 7000;
const int port_range_max = 9999;

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

int run_application(const std::string& fmu, const std::string& instanceName, bool local)
{

    if (!local) {
        const auto port = simple_socket::getAvailablePort(port_range_min, port_range_max);
        if (!port) {
            spdlog::error("Unable to locate free port number..");
            return UNHANDLED_ERROR;
        }

        try {
            simple_socket::TCPServer server(*port);
            spdlog::info("Serving proxy '{}' on port {}", instanceName, *port);
            // communication with parent process
            std::cout << "[proxyfmu] bind=" << std::to_string(*port) << std::endl;

            auto con = server.accept();
            spdlog::info("TCP Client connected");
            client_handler(std::move(con), fmu, instanceName);
        } catch (const std::exception& ex) {
            spdlog::error("Exception occurred: {}", ex.what());
            return UNHANDLED_ERROR;
        }
    } else {

        const auto fileHandle = instanceName + "_" + generate_uuid();

        try {
            simple_socket::UnixDomainServer server(fileHandle);
            spdlog::info("Serving proxy '{}' using file '{}'", instanceName, fileHandle);

            // communication with parent process
            std::cout << "[proxyfmu] bind=" << fileHandle << std::endl;

            auto con = server.accept();
            spdlog::info("Unix Domain Client connected");
            client_handler(std::move(con), fmu, instanceName);
        } catch (const std::exception& ex) {
            spdlog::error("Exception occurred: {}", ex.what());
            return UNHANDLED_ERROR;
        }
    }

    return SUCCESS;
}

int run_boot_application(const int port)
{

    spdlog::info("Boot application serving on port {}", port);

    boot_service_handler handler;
    simple_socket::TCPServer server(port);

    std::thread server_thread([&] {
        try {
            while (true) {
                const auto conn = server.accept();

                try {
                    std::vector<uint8_t> buffer(4);
                    conn->readExact(buffer);

                    const auto msgSize = simple_socket::decode_uint32(buffer);
                    buffer.resize(msgSize);
                    conn->readExact(buffer);

                    const auto root = flexbuffers::GetRoot(buffer.data(), msgSize).AsVector();

                    std::string fmuName = root[0].AsString().str();
                    std::string instanceName = root[1].AsString().str();
                    std::string data = root[2].AsString().str();

                    spdlog::info("Booting: {}::{}, file size={}", fmuName, instanceName, data.size());

                    const int16_t instance_port = handler.loadFromBinaryData(fmuName, instanceName, data);
                    flexbuffers::Builder fbb;
                    fbb.UInt(instance_port);
                    fbb.Finish();
                    conn->write(fbb.GetBuffer());

                    } catch (const std::exception& ex) {
                        spdlog::error("Exception occurred: {}", ex.what());
                    }
                }
            } catch (const std::exception&) {}

    });

    wait_for_input();

    server.close();
    server_thread.join();

    return SUCCESS;
}

int printHelp(const CLI::App& desc)
{
    std::cout << desc.help() << std::endl;
    return SUCCESS;
}

std::string versionString()
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

    app.set_version_flag("-v,--version", versionString());
    app.add_option("--fmu", "Location of the fmu to load.");
    app.add_option("--instanceName", "Name of the slave instance.");
    app.add_option("--local", "Running locally?");

    CLI::App* sub = app.add_subcommand("boot");
    sub->add_option("--port", "Specify the network port to be used.")->required();

    if (argc == 1) {
        return printHelp(app);
    }

    try {

        CLI11_PARSE(app, argc, argv);

        if (*sub) {

            const auto logger = spdlog::stdout_color_mt("proxyfmu");
            set_default_logger(logger);
            logger->set_level(spdlog::level::debug);

            const auto port = sub->get_option("--port")->as<int>();
            const auto status = run_boot_application(port);
            spdlog::shutdown();
            return status;

        }

        const auto local = app["--local"]->as<bool>();
        const auto instanceName = app["--instanceName"]->as<std::string>();

        const std::string logFile{"logs/" + instanceName + ".txt"};
        std::ofstream ofs(logFile, std::ofstream::out | std::ofstream::trunc);
        ofs.close();

        auto file_logger = spdlog::basic_logger_mt("proxyfmu", logFile);
        file_logger->set_level(spdlog::level::debug);
        file_logger->flush_on(spdlog::level::info);
        set_default_logger(std::move(file_logger));
        spdlog::flush_every(std::chrono::seconds(1));  // Flush every 1 second

        const auto fmu = app["--fmu"]->as<std::string>();
        const auto fmuPath = std::filesystem::path(fmu);
        if (!exists(fmuPath)) {
            spdlog::error("No such file: '{}'",absolute(fmuPath).string());
            return COMMANDLINE_ERROR;
        }

        spdlog::info("Got commandline arguments: --fmu '{}', --instanceName '{}', --local {}", fmu, instanceName, local);

        const auto status = run_application(fmu, instanceName, local);
        spdlog::shutdown();

        return status;

    } catch (const std::exception& e) {
        spdlog::error("Unhandled Exception reached the top of main: {}, application will now exit", e.what());
        return UNHANDLED_ERROR;
    }
}
