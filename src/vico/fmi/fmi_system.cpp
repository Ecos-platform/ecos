
#include "fmi_system.hpp"

using namespace vico;

void fmi_system::add_slave(const std::string& instanceName, std::unique_ptr<fmilibcpp::slave> slave)
{

    auto& md = slave->get_model_description();
    for (const auto& v : md.modelVariables) {
        std::string propertyName(instanceName + "." + v.name);
        if (v.is_integer()) {
            int_property p{
                [&] { return slave->get_integer(v.vr); },
                [&](auto value) { slave->set_integer({v.vr}, {value}); }};
            properties_[propertyName] = p;
        } else if (v.is_real()) {
            real_property p{
                [&] { return slave->get_real(v.vr); },
                [&](auto value) { slave->set_real({v.vr}, {value}); }};
        } else if (v.is_string()) {
            string_property p{
                [&] { return slave->get_string(v.vr); },
                [&](auto value) { slave->set_string({v.vr}, {value}); }};
            properties_[propertyName] = p;
        } else if (v.is_boolean()) {
            bool_property p{
                [&] { return slave->get_boolean(v.vr); },
                [&](auto value) { slave->set_boolean({v.vr}, {value}); }};
            properties_[propertyName] = p;
        }
    }

    slaves_.emplace_back(std::move(slave));
}

void fmi_system::init()
{

    for (auto& slave : slaves_) {
        slave->setup_experiment();
    }

    for (auto& slave : slaves_) {
        slave->enter_initialization_mode();
    }

    for (auto& slave : slaves_) {
        slave->exit_initialization_mode();
    }
}

void fmi_system::step(double currentTime, double stepSize)
{

    for (auto& slave : slaves_) {
        slave->step(currentTime, stepSize);
    }
}

void fmi_system::terminate()
{
    for (auto& slave : slaves_) {
        slave->terminate();
    }
}
