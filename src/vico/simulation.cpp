
#include "vico/simulation.hpp"

#include "vico/simulation_listener.hpp"

using namespace vico;

simulation::simulation(std::unique_ptr<simulation_structure> ss)
: ss_(std::move(ss))
{

}

void simulation::init(double startTime)
{
    for (auto& listener : listeners_) {
        listener->pre_init();
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

        //    algorithm_->step(currentTime);

        for (auto& listener : listeners_) {
            listener->post_step();
        }
    }
}

void simulation::terminate()
{
    for (auto& listener : listeners_) {
        listener->post_terminate();
    }
}

void simulation::add_listener(const std::shared_ptr<simulation_listener>& listener)
{
    listeners_.emplace_back(listener);
}
