
#include "fmi_system.hpp"

using namespace vico;

fmi_system::fmi_system(std::unique_ptr<algorithm> algorithm)
    : algorithm_(std::move(algorithm))
{ }

void fmi_system::add_slave(std::unique_ptr<fmilibcpp::slave> slave)
{

    auto& md = slave->get_model_description();
    fmilibcpp::slave* slave_pointer = slave.get();
    for (const auto& v : md.modelVariables) {
        std::string propertyName(slave->instanceName + "." + v.name);
        if (v.is_integer()) {
            int_property p{
                [&v, slave_pointer] { return slave_pointer->get_integer(v.vr); },
                [&v, slave_pointer](auto value) { slave_pointer->set_integer({v.vr}, {value}); }};
            properties_[propertyName] = p;
        } else if (v.is_real()) {
            real_property p{
                [&v, slave_pointer] { return slave_pointer->get_real(v.vr); },
                [&v, slave_pointer](auto value) { slave_pointer->set_real({v.vr}, {value}); }};
            properties_[propertyName] = p;
        } else if (v.is_string()) {
            string_property p{
                [&v, slave_pointer] { return slave_pointer->get_string(v.vr); },
                [&v, slave_pointer](auto value) { slave_pointer->set_string({v.vr}, {value}); }};
            properties_[propertyName] = p;
        } else if (v.is_boolean()) {
            bool_property p{
                [&v, slave_pointer] { return slave_pointer->get_boolean(v.vr); },
                [&v, slave_pointer](auto value) { slave_pointer->set_boolean({v.vr}, {value}); }};
            properties_[propertyName] = p;
        }
    }

    slaves_.emplace_back(std::move(slave));
    algorithm_->slave_added_internal(slaves_.back().get());
}

void fmi_system::init(double startTime)
{
    algorithm_->init(startTime);
}

void fmi_system::step(double currentTime, double stepSize)
{

    algorithm_->step(currentTime, stepSize);
}

void fmi_system::terminate()
{
    algorithm_->terminate();
}

fmi_system::~fmi_system()
{
    for (auto& slave : slaves_) {
        slave->freeInstance();
    }
}
