
#include "handlers/boot_service_handler.hpp"

#include "proxyfmu/process_helper.hpp"

#include <fstream>

using namespace proxyfmu::server;

namespace
{

void write_data(std::string const& fileName, std::string const& data)
{
    std::ofstream outFile(fileName, std::ios::binary);

    if (!outFile) {
        throw std::runtime_error("Unable to open file: " + fileName);
    }

    outFile.write(data.c_str(), data.size());

    if (!outFile) {
        throw std::runtime_error("Error during write to file: " + fileName);
    }
}

} // namespace

int32_t boot_service_handler::loadFromBinaryData(const std::string& fmuName, const std::string& instanceName, const std::string& data)
{
    auto tmp = std::make_unique<ecos::temp_dir>(fmuName);
    std::string fmuPath(tmp->path().string() + "/" + fmuName + ".fmu");

    write_data(fmuPath, data);

    std::promise<int> portPromise;
    auto t = std::make_unique<std::thread>(&proxyfmu::start_process, fmuPath, instanceName, std::ref(portPromise));
    processes_.emplace_back(std::move(t));
    dirs_.emplace_back(std::move(tmp));

    return portPromise.get_future().get();
}

boot_service_handler::~boot_service_handler()
{
    for (auto& t : processes_) {
        if (t->joinable()) t->join();
    }
}
