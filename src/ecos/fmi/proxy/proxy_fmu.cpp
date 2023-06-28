
#include "proxy_fmu.hpp"

#include "proxy_slave.hpp"

#include "ecos/fmi/fmu.hpp"

#include <memory>
#include <utility>


namespace ecos::proxyfmu
{

proxy_fmu::proxy_fmu(const std::filesystem::path& fmuPath, std::optional<remote_info> remote)
    : fmuPath_(fmuPath)
    , remote_(std::move(remote))
    , modelDescription_(fmi::loadFmu(fmuPath)->get_model_description())
{
    if (!exists(fmuPath)) throw std::runtime_error("No such file: " + std::filesystem::absolute(fmuPath).string() + "!");
}

const fmi::model_description& proxy_fmu::get_model_description() const
{
    return modelDescription_;
}

std::unique_ptr<fmi::slave> proxy_fmu::new_instance(const std::string& instanceName)
{
    return std::make_unique<proxy_slave>(fmuPath_, instanceName, modelDescription_, remote_);
}

} // namespace ecos::proxyfmu
