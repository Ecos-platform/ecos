
#include "vico/simulation.hpp"

#include "vico/simulation_listener.hpp"

using namespace vico;

simulation::simulation(double baseStepSize)
    : baseStepSize(baseStepSize)
{ }

void simulation::init(double startTime)
{
    for (auto& listener : listeners_) {
        listener->pre_init();
    }

    algorithm_->init(startTime);

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

        currentTime += baseStepSize;

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

void simulation::apply(const simulation_structure& ss)
{

    for (auto& model : ss.models_) {
        instances_.emplace_back(model.instantiate());
    }

    for (auto& c : ss.connections_) {

        std::visit([](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int_connection>) {

            } else if constexpr (std::is_same_v<T, real_connection>) {

            } else if constexpr (std::is_same_v<T, string_connection>) {

            } else if constexpr (std::is_same_v<T, bool_connection>) {
            }
        },
            c);
    }
}
