

#ifndef VICO_FMI_MODEL_INSTANCE_HPP
#define VICO_FMI_MODEL_INSTANCE_HPP

#include "vico/model_instance.hpp"

#include <fmilibcpp/buffered_slave.hpp>

namespace vico
{

class fmi_model_instance : public model_instance
{

public:
    fmi_model_instance(std::unique_ptr<fmilibcpp::slave> slave)
        : model_instance(slave->instanceName)
        , slave_(std::make_unique<fmilibcpp::buffered_slave>(std::move(slave)))
    {

        const auto name = slave_->instanceName;
        const auto& md = slave_->get_model_description();
        for (const auto& v : md.modelVariables) {
            std::string propertyName(v.name);
            if (v.is_integer()) {
                auto p = property_t<int>::create(
                    {slave_->instanceName, propertyName},
                    [&v, this] {
                        slave_->mark_for_reading(v.name);
                        return slave_->fmilibcpp::slave::get_integer(v.vr);
                    },
                    [&v, this](auto value) { slave_->set_integer({v.vr}, {value}); });
                properties_.add_int_property(propertyName, std::move(p));
            } else if (v.is_real()) {
                auto p = property_t<double>::create(
                    {slave_->instanceName, propertyName},
                    [&v, this] {
                        //                        slave_->mark_for_reading(v.name);
                        return slave_->fmilibcpp::slave::get_real(v.vr);
                    },
                    [&v, this](auto value) { slave_->set_real({v.vr}, {value}); });
                properties_.add_real_property(propertyName, std::move(p));
            } else if (v.is_string()) {
                auto p = property_t<std::string>::create(
                    {slave_->instanceName, propertyName},
                    [&v, this] {
                        slave_->mark_for_reading(v.name);
                        return slave_->fmilibcpp::slave::get_string(v.vr);
                    },
                    [&v, this](auto value) { slave_->set_string({v.vr}, {value}); });
                properties_.add_string_property(propertyName, std::move(p));
            } else if (v.is_boolean()) {
                auto p = property_t<bool>::create(
                    {slave_->instanceName, propertyName},
                    [&v, this] {
                        slave_->mark_for_reading(v.name);
                        return slave_->fmilibcpp::slave::get_boolean(v.vr);
                    },
                    [&v, this](auto value) { slave_->set_boolean({v.vr}, {value}); });
                properties_.add_bool_property(propertyName, std::move(p));
            } else {
                throw std::runtime_error("Assertion error");
            }
        }

        properties_.add_listener(std::make_unique<prop_lister>(*slave_));
    }
//
//    void set_values() override
//    {
//        slave_->transferCachedSets();
//    }
//
//    void get_values() override
//    {
//        slave_->receiveCachedGets();
//    }

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

    void reset() override
    {
        slave_->reset();
    }

private:
    std::unique_ptr<fmilibcpp::buffered_slave> slave_;

    struct prop_lister : property_listener
    {

        explicit prop_lister(fmilibcpp::buffered_slave& slave)
            : slave_(slave)
        { }

        void on_apply_sets() override
        {
            slave_.transferCachedSets();
        }
        void on_appy_gets() override
        {
            slave_.receiveCachedGets();
        }

    private:
        fmilibcpp::buffered_slave& slave_;
    };
};

} // namespace vico

#endif // VICO_FMI_MODEL_INSTANCE_HPP
