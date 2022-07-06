
#ifndef VICO_PROXY_MODEL_HPP
#define VICO_PROXY_MODEL_HPP

#include "proxyfmu/client/proxy_fmu.hpp"

#include "vico/fmi/fmi_model_instance.hpp"
#include "vico/model.hpp"

#include <filesystem>

namespace vico
{

class proxy_model : public model
{
public:
    proxy_model(const std::filesystem::path& fmuPath)
        : fmu_(fmuPath)
    {
    }

    std::unique_ptr<model_instance> instantiate(std::string instanceName)
    {
        return std::make_unique<fmi_model_instance>(std::move(fmu_.new_instance(instanceName)));
    }

private:
    proxyfmu::client::proxy_fmu fmu_;
};

} // namespace vico

#endif // VICO_PROXY_MODEL_HPP
