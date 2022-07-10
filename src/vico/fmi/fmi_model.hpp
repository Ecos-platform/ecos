
#ifndef VICO_FMI_MODEL_HPP
#define VICO_FMI_MODEL_HPP

#include "fmi_model_instance.hpp"
#include "fmilibcpp/fmu.hpp"

#include "vico/model.hpp"

#include <filesystem>

namespace vico
{

class fmi_model : public model
{

public:
    explicit fmi_model(const std::filesystem::path& fmuPath)
        : fmu_(fmilibcpp::loadFmu(fmuPath))
    { }

    [[nodiscard]] fmilibcpp::model_description get_model_description() const
    {
        return fmu_->get_model_description();
    }

    std::unique_ptr<model_instance> instantiate(const std::string& instanceName) override
    {
        return std::make_unique<fmi_model_instance>(fmu_->new_instance(instanceName));
    }

private:
    std::unique_ptr<fmilibcpp::fmu> fmu_;
};

} // namespace vico

#endif // VICO_FMI_MODEL_HPP
