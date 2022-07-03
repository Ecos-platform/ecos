
#include "vico/simulation.hpp"

#include "vico/simulation_listener.hpp"

using namespace vico;

simulation::simulation(std::unique_ptr<algorithm> algorithm)
    : algorithm_(std::move(algorithm))
{ }

void simulation::init(double startTime)
{
    if (!initialized) {

        initialized = true;

        for (auto& listener : listeners_) {
            listener->pre_init();
        }

        for (auto& [name, slave] : slaves_) {
            slave->setup_experiment(startTime);
            slave->enter_initialization_mode();
        }

        for (auto& [name, slave] : slaves_) {
            slave->transferCachedSets();
            slave->receiveCachedGets();
        }

        for (auto& [name, slave] : slaves_) {
            slave->exit_initialization_mode();
            slave->receiveCachedGets();
        }

        for (auto& listener : listeners_) {
            listener->post_init();
        }
    }
}

void simulation::step(unsigned int numStep)
{
    if (!initialized) { init(); }

    for (unsigned i = 0; i < numStep; i++) {

        for (auto& listener : listeners_) {
            listener->pre_step();
        }

        double newT = algorithm_->step(currentTime, [&](fmilibcpp::slave* slave) {

        });

        for (auto& [name, p] : properties_) {
            p->updateConnections();
        }

        currentTime = newT;

        for (auto& listener : listeners_) {
            listener->post_step();
        }

        num_iterations++;
    }
}

void simulation::terminate()
{
    for (auto& [name, slave] : slaves_) {
        slave->terminate();
    }

    for (auto& listener : listeners_) {
        listener->post_terminate();
    }
}

void simulation::add_listener(const std::shared_ptr<simulation_listener>& listener)
{
    listeners_.emplace_back(listener);
}

void simulation::add_slave(std::unique_ptr<fmilibcpp::slave> slave)
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
            properties_[propertyName] = p;
        } else if (v.is_real()) {
            auto p = property_t<double>::create(
                [&v, slave_pointer] {
                    return slave_pointer->get_real(v.vr);
                },
                [&v, slave_pointer](auto value) { slave_pointer->set_real({v.vr}, {value}); });
            properties_[propertyName] = p;
        } else if (v.is_string()) {
            auto p = property_t<std::string>::create(
                [&v, slave_pointer] { return slave_pointer->get_string(v.vr); },
                [&v, slave_pointer](auto value) { slave_pointer->set_string({v.vr}, {value}); });
            properties_[propertyName] = p;
        } else if (v.is_boolean()) {
            auto p = property_t<bool>::create(
                [&v, slave_pointer] { return slave_pointer->get_boolean(v.vr); },
                [&v, slave_pointer](auto value) { slave_pointer->set_boolean({v.vr}, {value}); });
            properties_[propertyName] = p;
        } else {
            throw std::runtime_error("Assertion error");
        }
    }

    algorithm_->slave_added_internal(buf.get());
    slaves_[name] = std::move(buf);
}

//
// property* simulation::get_property(const std::string& identifier)
//{
//    for (const auto& system : systems_) {
//        auto get = system->get_property(identifier);
//        if (get) return get;
//    }
//
//    return nullptr;
//}
