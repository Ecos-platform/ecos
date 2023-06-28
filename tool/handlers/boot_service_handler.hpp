
#ifndef PROXYFMU_BOOT_SERVICE_HANDLER_HPP
#define PROXYFMU_BOOT_SERVICE_HANDLER_HPP

#include "temp_dir.hpp"

#include <proxyfmu/thrift/BootService.h>

#include <memory>
#include <thread>
#include <vector>

namespace proxyfmu::server
{

class boot_service_handler : virtual public thrift::BootServiceIf
{

private:
    std::vector<std::unique_ptr<temp_dir>> dirs_;
    std::vector<std::unique_ptr<std::thread>> processes_;

public:
    int32_t loadFromBinaryData(const std::string& fmuName, const std::string& instanceName, const std::string& data) override;

    ~boot_service_handler() override;
};

} // namespace proxyfmu::server

#endif // PROXYFMU_BOOT_SERVICE_HANDLER_HPP
