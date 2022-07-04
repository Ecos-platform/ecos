
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

        for (auto& instance : instances_) {
            instance->setup_experiment(startTime);
            instance->enter_initialization_mode();
        }

        for (auto& instance : instances_) {
            instance->applySets();
            instance->applyGets();
        }

        for (auto& instance : instances_) {
            instance->exit_initialization_mode();
            instance->applyGets();
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

        double newT = algorithm_->step(currentTime, instances_);

        //        for (auto& [name, p] : properties_) {
        //            p->updateConnections();
        //        }

        for (auto& c : connections_) {
            c->transferData();
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
    for (auto& instance : instances_) {
        instance->terminate();
    }

    for (auto& listener : listeners_) {
        listener->post_terminate();
    }
}

void simulation::add_listener(const std::shared_ptr<simulation_listener>& listener)
{
    listeners_.emplace_back(listener);
}

model_instance* simulation::get_instance(const std::string& name)
{
    for (auto& instance : instances_) {
        if (instance->instanceName == name) {
            return instance.get();
        }
    }
    return nullptr;
}

void simulation::add_slave(std::unique_ptr<model_instance> instance)
{
    const auto name = instance->instanceName;
    if (get_instance(name)) {
        throw std::runtime_error("A model instance named '" + name + "' has already been added!");
    }

    instances_.emplace_back(std::move(instance));
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
