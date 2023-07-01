
#ifndef ECOS_FMI_MODEL_HPP
#define ECOS_FMI_MODEL_HPP

#include "fmi_model_instance.hpp"
#include "fmilibcpp/fmu.hpp"

#include "ecos/model.hpp"

#include <filesystem>

namespace ecos
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

    std::unique_ptr<model_instance> instantiate(const std::string& instanceName, std::optional<double> stepSizeHint) override
    {
        return std::make_unique<fmi_model_instance>(fmu_->new_instance(instanceName), stepSizeHint);
    }

private:
    std::unique_ptr<fmilibcpp::fmu> fmu_;
};

} // namespace ecos

#endif // ECOS_FMI_MODEL_HPP
