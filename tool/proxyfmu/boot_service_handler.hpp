
#ifndef PROXYFMU_BOOT_SERVICE_HANDLER_HPP
#define PROXYFMU_BOOT_SERVICE_HANDLER_HPP

#include "ecos/util/temp_dir.hpp"

#include "proxyfmu/process_helper.hpp"

#include <memory>
#include <thread>
#include <vector>
#include <fstream>

namespace ecos::proxy
{

class boot_service_handler
{

public:
    int16_t loadFromBinaryData(const std::string& fmuName, const std::string& instanceName, const std::string& data)
    {
        auto tmp = std::make_unique<ecos::temp_dir>(fmuName);
        std::string fmuPath(tmp->path().string() + "/" + fmuName + ".fmu");

        write_data(fmuPath, data);

        std::promise<int> portPromise;
        std::thread t(&start_process, fmuPath, instanceName, std::ref(portPromise));
        processes_.emplace_back(std::move(t));
        dirs_.emplace_back(std::move(tmp));

        return static_cast<int16_t>(portPromise.get_future().get());
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

    static void write_data(std::string const& fileName, std::string const& data)
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

};

} // namespace proxyfmu::server

#endif // PROXYFMU_BOOT_SERVICE_HANDLER_HPP
