
#ifndef ECOS_FMI_MODEL_INSTANCE_HPP
#define ECOS_FMI_MODEL_INSTANCE_HPP

#include "fmilibcpp/buffered_slave.hpp"

#include "ecos/model_instance.hpp"
#include <ecos/logger/logger.hpp>

namespace ecos
{

class fmi_model_instance : public model_instance
{

public:
    explicit fmi_model_instance(std::unique_ptr<fmilibcpp::slave> slave, std::optional<double> stepSizeHint)
        : model_instance(slave->instanceName, stepSizeHint)
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
                        vrBuf[0] = v.vr;
                        slave_->get_integer(vrBuf, iBuf);
                        return iBuf.back();
                    },
                    [&v, this](auto value) {
                        vrBuf[0] = v.vr;
                        iBuf[0] = value;
                        slave_->set_integer(vrBuf, iBuf);
                    });
                properties_.add_int_property(propertyName, std::move(p));
            } else if (v.is_real()) {
                auto p = property_t<double>::create(
                    {slave_->instanceName, propertyName},
                    [&v, this] {
                        vrBuf[0] = v.vr;
                        slave_->get_real(vrBuf, rBuf);
                        return rBuf.back();
                    },
                    [&v, this](auto value) {
                        vrBuf[0] = v.vr;
                        rBuf[0] = value;
                        slave_->set_real(vrBuf, rBuf);
                    });
                properties_.add_real_property(propertyName, std::move(p));
            } else if (v.is_string()) {
                auto p = property_t<std::string>::create(
                    {slave_->instanceName, propertyName},
                    [&v, this] {
                        vrBuf[0] = v.vr;
                        slave_->get_string(vrBuf, sBuf);
                        return sBuf.back();
                    },
                    [&v, this](auto& value) {
                        vrBuf[0] = v.vr;
                        sBuf[0] = value;
                        slave_->set_string(vrBuf, sBuf);
                    });
                properties_.add_string_property(propertyName, std::move(p));
            } else if (v.is_boolean()) {
                auto p = property_t<bool>::create(
                    {slave_->instanceName, propertyName},
                    [&v, this] {
                        vrBuf[0] = v.vr;
                        slave_->get_boolean(vrBuf, bBuf);
                        return bBuf.back();
                    },
                    [&v, this](auto value) {
                        vrBuf[0] = v.vr;
                        bBuf[0] = value;
                        slave_->set_boolean(vrBuf, bBuf);
                        return bBuf.back();
                    });
                properties_.add_bool_property(propertyName, std::move(p));
            } else {
                throw std::runtime_error("Assertion error");
            }
        }

        properties_.add_listener(std::make_unique<prop_lister>(*slave_));
    }

    void enter_initialization_mode(double start) override
    {
        slave_->enter_initialization_mode(start);
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
    std::vector<fmilibcpp::value_ref> vrBuf = std::vector<fmilibcpp::value_ref>(1);
    std::vector<double> rBuf = std::vector<double>(1);
    std::vector<int> iBuf = std::vector<int>(1);
    std::vector<bool> bBuf = std::vector<bool>(1);
    std::vector<std::string> sBuf = std::vector<std::string>(1);
    std::unique_ptr<fmilibcpp::buffered_slave> slave_;

    struct prop_lister : property_listener
    {

        explicit prop_lister(fmilibcpp::buffered_slave& slave)
            : slave_(slave)
        { }

        void post_sets() override
        {
            slave_.transferCachedSets();
        }
        void pre_gets() override
        {
            slave_.receiveCachedGets();
        }

    private:
        fmilibcpp::buffered_slave& slave_;
    };
};

} // namespace ecos

#endif // ECOS_FMI_MODEL_INSTANCE_HPP
