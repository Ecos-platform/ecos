

#ifndef VICO_FMI_MODEL_INSTANCE_HPP
#define VICO_FMI_MODEL_INSTANCE_HPP

#include "../model_instance.hpp"

#include <fmilibcpp/slave.hpp>

namespace vico
{

class fmi_model_instance : public model_instance
{

public:
    fmi_model_instance(std::unique_ptr<fmilibcpp::slave> slave)
        : model_instance(slave->instanceName)
        , slave_(std::move(slave))
    {

        const auto name = slave_->instanceName;
        const auto& md = slave_->get_model_description();
        for (const auto& v : md.modelVariables) {
            std::string propertyName(v.name);
            if (v.is_integer()) {
                auto p = property_t<int>::create(
                    [&v, this] { return slave_->get_integer(v.vr); },
                    [&v, this](auto value) { slave_->set_integer({v.vr}, {value}); });
                properties_[propertyName] = std::move(p);
            } else if (v.is_real()) {
                auto p = property_t<double>::create(
                    [&v, this] {
                        return slave_->get_real(v.vr);
                    },
                    [&v, this](auto value) { slave_->set_real({v.vr}, {value}); });
                properties_[propertyName] = std::move(p);
            } else if (v.is_string()) {
                auto p = property_t<std::string>::create(
                    [&v, this] { return slave_->get_string(v.vr); },
                    [&v, this](auto value) { slave_->set_string({v.vr}, {value}); });
                properties_[propertyName] = std::move(p);
            } else if (v.is_boolean()) {
                auto p = property_t<bool>::create(
                    [&v, this] { return slave_->get_boolean(v.vr); },
                    [&v, this](auto value) { slave_->set_boolean({v.vr}, {value}); });
                properties_[propertyName] = std::move(p);
            } else {
                throw std::runtime_error("Assertion error");
            }
        }
    }

    void setup_experiment(double start) override
    {
        slave_->setup_experiment(start);
    }

    void enter_initialization_mode() override
    {
        slave_->enter_initialization_mode();
    }

    void exit_initialization_mode() override
    {
        slave_->exit_initialization_mode();
    }

    void step(double currentTime, double stepSize) override
    {
        slave_->step(currentTime, stepSize);
    }

    void terminate() override
    {
        slave_->terminate();
    }

private:
    std::unique_ptr<fmilibcpp::slave> slave_;
};

} // namespace vico

#endif // VICO_FMI_MODEL_INSTANCE_HPP