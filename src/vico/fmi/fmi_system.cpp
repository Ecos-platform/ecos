
#include "vico/fmi/fmi_system.hpp"

#include <fmilibcpp/buffered_slave.hpp>

using namespace vico;

struct fmi_system::Impl
{

    explicit Impl(std::unique_ptr<algorithm> algorithm)
        : algorithm_(std::move(algorithm))
    { }

    void add_slave(std::unique_ptr<fmilibcpp::slave> slave, std::unordered_map<std::string, std::shared_ptr<property>>& properties)
    {
        const auto name = slave->instanceName;
        if (slaves_.count(name)) {
            throw std::runtime_error("A slave named '" + name + "' has already been added!");
        }

        auto& md = slave->get_model_description();
        auto buf = std::make_unique<fmilibcpp::buffered_slave>(std::move(slave));
        fmilibcpp::slave* slave_pointer = buf.get();
        for (const auto& v : md.modelVariables) {
            std::string propertyName(name + "." + v.name);
            if (v.is_integer()) {
                auto p = property_t<int>::create(
                    [&v, slave_pointer] { return slave_pointer->get_integer(v.vr); },
                    [&v, slave_pointer](auto value) { slave_pointer->set_integer({v.vr}, {value}); });
                properties[propertyName] = p;
            } else if (v.is_real()) {
                auto p = property_t<double>::create(
                    [&v, slave_pointer] {
                        return slave_pointer->get_real(v.vr);
                    },
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
                throw std::runtime_error("Assertion error");
            }
        }

        algorithm_->slave_added_internal(buf.get());
        slaves_[name] = std::move(buf);
    }

    void init(double startTime)
    {
        algorithm_->init(startTime);
    }

    void step(double currentTime, double stepSize, std::unordered_map<std::string, std::shared_ptr<property>>& properties)
    {
        algorithm_->step(currentTime, stepSize, [&](fmilibcpp::slave* slave) {

        });

        for (auto& [name, p] : properties) {
            p->updateConnections();
        }
    }

    void terminate()
    {
        algorithm_->terminate();
    }

    ~Impl() = default;

private:
    std::unique_ptr<algorithm> algorithm_;
    std::unordered_map<std::string, std::unique_ptr<fmilibcpp::buffered_slave>> slaves_;
};

fmi_system::fmi_system(std::unique_ptr<algorithm> algorithm)
    : pimpl_{std::make_unique<Impl>(std::move(algorithm))}
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

vico::fmi_system::~fmi_system() = default;
