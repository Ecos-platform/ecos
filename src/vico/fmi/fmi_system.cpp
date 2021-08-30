
#include "fmi_system.hpp"

using namespace vico;

namespace
{

//struct cache
//{
//
//    std::unordered_map<fmilibcpp::value_ref, int> integerSetCache;
//    std::unordered_map<fmilibcpp::value_ref, double> realSetCache;
//    std::unordered_map<fmilibcpp::value_ref, std::string> stringSetCache;
//    std::unordered_map<fmilibcpp::value_ref, bool> boolSetCache;
//};


} // namespace

struct fmi_system::Impl
{

    explicit Impl(std::unique_ptr<algorithm> algorithm)
        : algorithm_(std::move(algorithm))
    { }

    void add_slave(std::unique_ptr<fmilibcpp::slave> slave, std::unordered_map<std::string, std::shared_ptr<property>>& properties)
    {
        auto& md = slave->get_model_description();
        fmilibcpp::slave* slave_pointer = slave.get();
        for (const auto& v : md.modelVariables) {
            std::string propertyName(slave->instanceName + "." + v.name);
            if (v.is_integer()) {
                auto p = property_t<int>::create(
                    [&v, slave_pointer] { return slave_pointer->get_integer(v.vr); },
                    [&v, slave_pointer](auto value) { slave_pointer->set_integer({v.vr}, {value}); });
                properties[propertyName] = p;
            } else if (v.is_real()) {
                auto p = property_t<double>::create(
                    [&v, slave_pointer] { return slave_pointer->get_real(v.vr); },
                    [&v, slave_pointer](auto value) { slave_pointer->set_real({v.vr}, {value}); });
                properties[propertyName] = p;
            } else if (v.is_string()) {
                auto p = property_t<std::string>::create(
                    [&v, slave_pointer] { return slave_pointer->get_string(v.vr); },
                    [&v, slave_pointer](auto value) { slave_pointer->set_string({v.vr}, {value}); });
                properties[propertyName] = p;
            } else if (v.is_boolean()) {
                auto p = property_t<bool>::create(
                    [&v, slave_pointer] { return slave_pointer->get_boolean(v.vr); },
                    [&v, slave_pointer](auto value) { slave_pointer->set_boolean({v.vr}, {value}); });
                properties[propertyName] = p;
            } else {
                throw std::runtime_error("");
            }
        }

        algorithm_->slave_added_internal(slave.get());
        slaves_[slave->instanceName] = std::move(slave);
    }

    void init(double startTime) {
        algorithm_->init(startTime);
    }

    void step(double currentTime, double stepSize, std::unordered_map<std::string, std::shared_ptr<property>>& properties) {
        algorithm_->step(currentTime, stepSize, [&](fmilibcpp::slave* slave) {
            for (auto& [name, p] : properties) {
                p->updateConnections();
            }
        });
    }

    void terminate() {
        algorithm_->terminate();
    }

    ~Impl() {
        for (auto& [_, slave] : slaves_) {
            slave->freeInstance();
        }
    }

private:
    std::unique_ptr<algorithm> algorithm_;
    std::unordered_map<std::string, std::unique_ptr<fmilibcpp::slave>> slaves_;
};

fmi_system::fmi_system(std::unique_ptr<algorithm> algorithm)
    : pimpl_(new Impl(std::move(algorithm)))
{ }

void fmi_system::add_slave(std::unique_ptr<fmilibcpp::slave> slave)
{

    pimpl_->add_slave(std::move(slave), properties_);
}

void fmi_system::init(double startTime)
{
    pimpl_->init(startTime);
}

void fmi_system::step(double currentTime, double stepSize)
{
    pimpl_->step(currentTime, stepSize, properties_);
}

void fmi_system::terminate()
{
    pimpl_->terminate();
}
