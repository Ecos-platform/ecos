
#ifndef PROXYFMU_BOOT_SERVICE_HANDLER_HPP
#define PROXYFMU_BOOT_SERVICE_HANDLER_HPP

#include "ecos/util/temp_dir.hpp"

#include <memory>
#include <thread>
#include <vector>

namespace proxyfmu::server
{

class boot_service_handler
{

private:
    std::vector<std::unique_ptr<ecos::temp_dir>> dirs_;
    std::vector<std::thread> processes_;

public:
    int16_t loadFromBinaryData(const std::string& fmuName, const std::string& instanceName, const std::string& data);

    ~boot_service_handler();
};

} // namespace proxyfmu::server

#endif // PROXYFMU_BOOT_SERVICE_HANDLER_HPP
