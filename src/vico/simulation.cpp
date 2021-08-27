
#include "vico/simulation.hpp"

#include "vico/simulation_listener.hpp"

using namespace vico;

simulation::simulation(double baseStepSize)
    : baseStepSize(baseStepSize)
{ }

void simulation::add_system(std::unique_ptr<system> system)
{
    systems_.emplace_back(std::move(system));
}

void simulation::add_connection(std::unique_ptr<connection> c)
{
    connections_.emplace_back(std::move(c));
}


void simulation::init(double startTime)
{
    for (auto& listener : listeners_) {
        listener->pre_init();
    }

    for (auto& system : systems_) {
        system->init();
    }

    for (auto& listener : listeners_) {
        listener->post_init();
    }
}

void simulation::step(unsigned int numStep)
{

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

void simulation::updateConnections()
{
    for (auto& c : connections_) {
        c->transferData();
    }
}
