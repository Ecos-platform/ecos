
#ifndef PROXYFMU_BOOT_SERVICE_HANDLER_HPP
#define PROXYFMU_BOOT_SERVICE_HANDLER_HPP

#include "util/temp_dir.hpp"

#include "proxyfmu/process_helper.hpp"

#include <fstream>
#include <memory>
#include <thread>
#include <vector>

namespace ecos::proxy
{

class boot_service_handler
{

public:
    int16_t loadFromBinaryData(const std::string& fmuName, const std::string& instanceName, const std::vector<uint8_t>& data)
    {
        auto tmp = std::make_unique<temp_dir>(fmuName);
        std::string fmuPath(tmp->path().string() + "/" + fmuName + ".fmu");

        write_data(fmuPath, data);

        std::promise<std::string> portPromise;
        std::thread t(&start_process, fmuPath, instanceName, std::ref(portPromise), false);
        processes_.emplace_back(std::move(t));
        dirs_.emplace_back(std::move(tmp));

        std::string portStr = portPromise.get_future().get();
        const auto port = static_cast<int16_t>(std::stoi(portStr));

        return port;
    }

    ~boot_service_handler()
    {
        for (auto& t : processes_) {
            if (t.joinable()) t.join();
        }
    }


private:
    std::vector<std::unique_ptr<temp_dir>> dirs_;
    std::vector<std::thread> processes_;

    static void write_data(std::string const& fileName, const std::vector<uint8_t>& data)
    {
        std::ofstream outFile(fileName, std::ios::binary);

        if (!outFile) {
            throw std::runtime_error("Unable to open file: " + fileName);
        }

        outFile.write(reinterpret_cast<const char*>(data.data()), data.size());

        if (!outFile) {
            throw std::runtime_error("Error during write to file: " + fileName);
        }
    }
};

} // namespace ecos::proxy

#endif // PROXYFMU_BOOT_SERVICE_HANDLER_HPP
