
#ifndef VICO_PROXY_MODEL_HPP
#define VICO_PROXY_MODEL_HPP

#include "proxyfmu/client/proxy_fmu.hpp"

#include "vico/fmi/fmi_model_instance.hpp"
#include "vico/model.hpp"

#include <filesystem>
#include <ssp/util/temp_dir.hpp>

namespace vico
{

class proxy_model : public model
{
public:
    explicit proxy_model(const std::filesystem::path& fmuPath,std::shared_ptr<ssp::temp_dir> tempDir)
        : fmu_(fmuPath), tempDir_(std::move(tempDir))
    {
    }

    std::unique_ptr<model_instance> instantiate(const std::string& instanceName) override
    {
        return std::make_unique<fmi_model_instance>(std::move(fmu_.new_instance(instanceName)));
    }

private:
    proxyfmu::client::proxy_fmu fmu_;
    std::shared_ptr<ssp::temp_dir> tempDir_;
};

} // namespace vico

#endif // VICO_PROXY_MODEL_HPP
