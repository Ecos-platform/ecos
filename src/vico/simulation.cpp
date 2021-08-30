
#include "vico/simulation.hpp"

#include "vico/simulation_listener.hpp"

using namespace vico;

simulation::simulation(double baseStepSize)
    : baseStepSize(baseStepSize)
{ }

void simulation::add_system(std::unique_ptr<vico::system> system)
{
    systems_.emplace_back(std::move(system));
}

void simulation::init(double startTime)
{

    if (!initialized) {
        initialized = true;
        for (auto& listener : listeners_) {
            listener->pre_init();
        }

        for (auto& system : systems_) {
            system->init(startTime);
        }

        for (auto& listener : listeners_) {
            listener->post_init();
        }
    }
}

void simulation::step(unsigned int numStep)
{

    if (!initialized) init();

    for (unsigned i = 0; i < numStep; i++) {

        for (auto& listener : listeners_) {
            listener->pre_step();
        }

        for (auto& system : systems_) {
            system->step(currentTime, baseStepSize);
        }

        currentTime += baseStepSize;

        for (auto& listener : listeners_) {
            listener->post_step();
        }

        num_iterations++;
    }
}

void simulation::terminate()
{
    for (auto& system : systems_) {
        system->terminate();
    }

    for (auto& listener : listeners_) {
        listener->post_terminate();
    }
}

void simulation::add_listener(const std::shared_ptr<simulation_listener>& listener)
{
    listeners_.emplace_back(listener);
}

property* simulation::get_property(const std::string& identifier)
{

    for (const auto& system : systems_) {
        auto get = system->get_property(identifier);
        if (get) return get;
    }

    return nullptr;
}
