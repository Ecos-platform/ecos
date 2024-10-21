
#ifndef ECOS_PROXY_MODEL_HPP
#define ECOS_PROXY_MODEL_HPP

#include "ecos/fmi/fmi_model_instance.hpp"
#include "ecos/model.hpp"

#include "proxyfmu/proxy_fmu.hpp"

#include <filesystem>
#include <utility>

namespace ecos
{

class proxy_model : public model
{
public:
    explicit proxy_model(const std::filesystem::path& fmuPath, std::optional<proxy::remote_info> remote = std::nullopt)
        : fmu_(fmuPath, std::move(remote))
    {
    }

    std::unique_ptr<model_instance> instantiate(const std::string& instanceName, std::optional<double> stepSizeHint) override
    {
        return std::make_unique<fmi_model_instance>(std::move(fmu_.new_instance(instanceName)), stepSizeHint);
    }

private:
    proxy::proxy_fmu fmu_;
};

} // namespace ecos

#endif // ECOS_PROXY_MODEL_HPP
