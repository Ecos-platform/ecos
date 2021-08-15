
#include "vico/simulation.hpp"

#include "vico/simulation_listener.hpp"

using namespace vico;


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

void simulation::reset()
{
}

void simulation::add_listener(const std::shared_ptr<simulation_listener>& listener)
{
    listeners_.emplace_back(listener);
}
