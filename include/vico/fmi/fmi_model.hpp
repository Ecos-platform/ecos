
#ifndef VICO_FMI_MODEL_HPP
#define VICO_FMI_MODEL_HPP

#include "fmilibcpp/fmu.hpp"

#include "vico/fmi/fmi_model_instance.hpp"
#include "vico/model.hpp"

namespace vico
{

class fmi_model : public model
{

public:

    explicit fmi_model(const std::string& fmuPath): fmu_(fmilibcpp::loadFmu(fmuPath)) {}

    std::unique_ptr<model_instance> instantiate(std::string instanceName) override
    {
        return std::make_unique<fmi_model_instance>(fmu_->new_instance(instanceName));
    }

private:
    std::unique_ptr<fmilibcpp::fmu> fmu_;
};

} // namespace vico

#endif // VICO_FMI_MODEL_HPP
