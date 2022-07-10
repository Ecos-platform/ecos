
#include "vico/simulation.hpp"

#include "spdlog/spdlog.h"

#include "vico/listeners/simulation_listener.hpp"
#include <execution>

using namespace vico;

simulation::simulation(std::unique_ptr<algorithm> algorithm)
    : algorithm_(std::move(algorithm))
{ }

void simulation::init(std::optional<double> startTime, std::optional<std::string> parameterSet)
{
    if (!initialized_) {

        initialized_ = true;
        spdlog::debug("Initializing simulation..");

        for (auto& listener : listeners_) {
            listener->pre_init(*this);
        }

        int parameterSetAppliedCount = 0;
        for (auto& instance : instances_) {
            double start = startTime.value_or(0);
            if (start < 0) {
                throw std::runtime_error("Explicitly defined startTime must be greater than 0!");
            }
            instance->setup_experiment(start);
            instance->enter_initialization_mode();
            if (parameterSet) {
                if (instance->apply_parameter_set(*parameterSet)) {
                    ++parameterSetAppliedCount;
                }
            }
        }
        if (parameterSet) {
            spdlog::debug("Parameterset '{}' applied to {} instances", *parameterSet, parameterSetAppliedCount);
        }

        for (unsigned i = 0; i < instances_.size(); ++i) {
            for (auto& instance : instances_) {
                instance->get_properties().apply_sets();
                instance->get_properties().applyGets();
            }
            for (auto& c : connections_) {
                c->transferData();
            }
        }

        for (auto& instance : instances_) {
            instance->exit_initialization_mode();
            instance->get_properties().applyGets();
        }

        for (auto& c : connections_) {
            c->transferData();
        }

        for (auto& instance : instances_) {
            instance->get_properties().apply_sets();
            instance->get_properties().applyGets();
        }

        for (auto& listener : listeners_) {
            listener->post_init(*this);
        }

        spdlog::debug("Initialized.");
    }
}

void simulation::step(unsigned int numStep)
{
    if (!initialized_) {
        throw std::runtime_error("init() has not been invoked!");
    }

    for (unsigned i = 0; i < numStep; i++) {

        for (auto& listener : listeners_) {
            listener->pre_step(*this);
        }

        double newT = algorithm_->step(currentTime_, instances_);

        for (auto& c : connections_) {
            c->transferData();
        }

        std::for_each(std::execution::par, instances_.begin(), instances_.end(), [](auto& instance){
            instance->get_properties().apply_sets();
            instance->get_properties().applyGets();
        });

        currentTime_ = newT;

        for (auto& listener : listeners_) {
            listener->post_step(*this);
        }

        num_iterations_++;
    }
}

void simulation::step_until(double t)
{
    if (t <= currentTime_) {
        spdlog::warn("Input time {} is not greater than the current simulation time. Simulation will not progress.", t, currentTime_);
    } else {
        while (currentTime_ < t) {
            step();
        }
    }
}

void simulation::step_for(double t)
{
    double newT = currentTime_ + t;
    step_until(newT);
}

void simulation::terminate()
{
    spdlog::debug("Terminating simulation..");

    for (auto& instance : instances_) {
        instance->terminate();
    }

    for (auto& listener : listeners_) {
        listener->post_terminate(*this);
    }

    spdlog::debug("Terminated.");
}

void simulation::reset()
{
    spdlog::debug("Resetting simulation at t={}", time());
    for (auto& instance : instances_) {
        instance->reset();
    }
    initialized_ = false;
}

void simulation::add_listener(std::unique_ptr<simulation_listener> listener)
{
    listeners_.emplace_back(std::move(listener));
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
