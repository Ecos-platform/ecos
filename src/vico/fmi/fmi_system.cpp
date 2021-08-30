
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
            auto p = property_t<int>::create(
                [&v, slave_pointer] { return slave_pointer->get_integer(v.vr); },
                [&v, slave_pointer](auto value) { slave_pointer->set_integer({v.vr}, {value}); });
            properties_[propertyName] = p;
        } else if (v.is_real()) {
           auto p = property_t<double>::create(
                [&v, slave_pointer] { return slave_pointer->get_real(v.vr); },
                [&v, slave_pointer](auto value) { slave_pointer->set_real({v.vr}, {value}); });
            properties_[propertyName] = p;
        } else if (v.is_string()) {
           auto p = property_t<std::string>::create(
                [&v, slave_pointer] { return slave_pointer->get_string(v.vr); },
                [&v, slave_pointer](auto value) { slave_pointer->set_string({v.vr}, {value}); } );
            properties_[propertyName] = p;
        } else if (v.is_boolean()) {
            auto p = property_t<bool>::create(
                [&v, slave_pointer] { return slave_pointer->get_boolean(v.vr); },
                [&v, slave_pointer](auto value) { slave_pointer->set_boolean({v.vr}, {value}); });
            properties_[propertyName] = p;
        }
    }

    algorithm_->slave_added_internal(slave.get());
    slaves_[slave->instanceName] = std::move(slave);
}

void fmi_system::init(double startTime)
{
    algorithm_->init(startTime);
}

void fmi_system::step(double currentTime, double stepSize)
{

    algorithm_->step(currentTime, stepSize, [this](fmilibcpp::slave* slave){
        for (auto &[name, p] : properties_) {
//            if (name.find(slave->instanceName) != std::string::npos) {
                p->updateConnections();
//            }
        }
    });
}

void fmi_system::terminate()
{
    algorithm_->terminate();
}

fmi_system::~fmi_system()
{
    for (auto& [_, slave] : slaves_) {
        slave->freeInstance();
    }
}
