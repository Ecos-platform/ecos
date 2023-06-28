
#include "proxy_slave.hpp"

#include "proxy_fmu.hpp"

#include "ecos/fmi/fmu.hpp"

#include <memory>
#include <utility>


namespace ecos::proxyfmu
{

proxy_fmu::proxy_fmu(const std::filesystem::path& fmuPath, std::optional<remote_info> remote)
    : fmuPath_(fmuPath)
    , remote_(std::move(remote))
    , modelDescription_(fmilibcpp::loadFmu(fmuPath)->get_model_description())
{
    if (!exists(fmuPath)) throw std::runtime_error("No such file: " + filesystem::absolute(fmuPath).string() + "!");
}

const fmilibcpp::model_description& proxy_fmu::get_model_description() const
{
    return modelDescription_;
}

std::unique_ptr<fmilibcpp::slave> proxy_fmu::new_instance(const std::string& instanceName)
{
    return std::make_unique<proxy_slave>(fmuPath_, instanceName, modelDescription_, remote_);
}

} // namespace proxyfmu::client
