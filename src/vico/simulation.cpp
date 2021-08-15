
#include "vico/simulation.hpp"

#include "vico/simulation_listener.hpp"

using namespace vico;


simulation::simulation(std::unique_ptr<algorithm> algorithm)
    : algorithm_(std::move(algorithm))
{
}


void simulation::init(double startTime)
{
}

void simulation::step(unsigned int numStep)
{
    for (auto& listener : listeners_) {
        listener->pre_step();
    }

//    algorithm_->step(currentTime);

    for (auto& listener : listeners_) {
        listener->post_step();
    }
}

void simulation::terminate()
{
}

void simulation::add_listener(const std::shared_ptr<simulation_listener>& listener)
{
    listeners_.emplace_back(listener);
}
