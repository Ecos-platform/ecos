
#include "ecos/simulation.hpp"

#include "ecos/listeners/simulation_listener.hpp"
#include "ecos/logger/logger.hpp"

#include <execution>
#include <ranges>

using namespace ecos;

simulation::simulation(std::unique_ptr<algorithm> algorithm)
    : algorithm_(std::move(algorithm))
{ }

void simulation::init(std::optional<double> startTime, const std::optional<std::string>& parameterSet)
{
    if (!initialized_) {

        initialized_ = true;
        log::debug("Initializing simulation..");

        for (auto l = listeners_; const auto& listener : l | std::views::values) {
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
            log::debug("Parameterset '{}' applied to {} instances", *parameterSet, parameterSetAppliedCount);
        }

        scenario_.runInitActions();

        for (unsigned i = 0; i < instances_.size(); ++i) {
            for (const auto& instance : instances_) {
                instance->get_properties().apply_sets();
                instance->get_properties().apply_gets();
            }
            for (const auto& c : connections_) {
                c->transferData();
            }
        }

        for (const auto& instance : instances_) {
            instance->exit_initialization_mode();
            instance->get_properties().apply_gets();
        }

        for (const auto& c : connections_) {
            c->transferData();
        }

        for (const auto& instance : instances_) {
            instance->get_properties().apply_sets();
            instance->get_properties().apply_gets();

            algorithm_->model_instance_added(instance.get());
        }

        for (auto l = listeners_; const auto& listener : listeners_ | std::views::values) {
            listener->post_init(*this);
        }

        log::debug("Initialized.");
    }
}

double simulation::step(unsigned int numStep)
{
    if (!initialized_) {
        throw std::runtime_error("init() has not been invoked!");
    }

    double newT{};
    for (unsigned i = 0; i < numStep; ++i) {

        for (auto l = listeners_; const auto& listener : l | std::views::values) {
            listener->pre_step(*this);
        }

        scenario_.apply(time());

        newT = algorithm_->step(currentTime_);

        for (const auto& c : connections_) {
            c->transferData();
        }

        std::for_each(std::execution::par, instances_.begin(), instances_.end(), [](auto& instance) {
            instance->get_properties().apply_sets();
            instance->get_properties().apply_gets();
        });

        currentTime_ = newT;
        ++num_iterations_;

        for (auto l = listeners_; const auto& listener : l | std::views::values) {
            listener->post_step(*this);
        }
    }

    return newT;
}

void simulation::step_until(double t)
{
    if (t <= currentTime_) {
        log::warn("Input time {} is not greater than the current simulation time {}. Simulation will not progress.", t, currentTime_);
    } else {
        while (currentTime_ < t) {
            step();
        }
    }
}

void simulation::step_for(double t)
{
    const double newT = currentTime_ + t;
    step_until(newT);
}

void simulation::terminate()
{
    if (!terminated_) {
        terminated_ = true;

        log::debug("Terminating simulation..");

        for (auto& instance : instances_) {
            instance->terminate();

            algorithm_->model_instance_removed(instance.get());
        }

        for (auto l = listeners_; const auto& listener : l | std::views::values) {
            listener->post_terminate(*this);
        }

        log::debug("Terminated.");
    }
}

void simulation::reset()
{
    log::debug("Resetting simulation at t={}", time());
    for (const auto& instance : instances_) {
        instance->reset();
    }
    scenario_.reset();
    currentTime_ = 0;
    num_iterations_ = 0;
    initialized_ = false;
}

void simulation::add_listener(const std::string& name, std::shared_ptr<simulation_listener> listener)
{
    if (listeners_.contains(name)) {
        log::warn("A listener named {} already exists..", name);
    } else {
        listeners_[name] = std::move(listener);
    }
}

void simulation::remove_listener(const std::string& name)
{
    log::debug("Removing listener named {}", name);
    listeners_.erase(name);
}

model_instance* simulation::get_instance(const std::string& name) const
{
    for (auto& instance : instances_) {
        if (instance->instanceName() == name) {
            return instance.get();
        }
    }
    return nullptr;
}

void simulation::add_slave(std::unique_ptr<model_instance> instance)
{
    if (const auto name = instance->instanceName(); get_instance(name)) {
        throw std::runtime_error("A model instance named '" + name + "' has already been added!");
    }

    instances_.emplace_back(std::move(instance));
}

real_connection* simulation::make_real_connection(const variable_identifier& source, const variable_identifier& sink)
{
    const auto p1 = get_real_property(source);
    if (!p1) throw std::runtime_error("No such real property: " + source.str());
    const auto p2 = get_real_property(sink);
    if (!p2) throw std::runtime_error("No such real property: " + sink.str());

    connections_.emplace_back(std::make_unique<real_connection>(p1, p2));
    return dynamic_cast<real_connection*>(connections_.back().get());
}

int_connection* simulation::make_int_connection(const variable_identifier& source, const variable_identifier& sink)
{
    const auto p1 = get_int_property(source);
    if (!p1) throw std::runtime_error("No such int property: " + source.str());
    const auto p2 = get_int_property(sink);
    if (!p2) throw std::runtime_error("No such int property: " + sink.str());

    connections_.emplace_back(std::make_unique<int_connection>(p1, p2));
    return dynamic_cast<int_connection*>(connections_.back().get());
}

bool_connection* simulation::make_bool_connection(const variable_identifier& source, const variable_identifier& sink)
{
    const auto p1 = get_bool_property(source);
    if (!p1) throw std::runtime_error("No such bool property: " + source.str());
    const auto p2 = get_bool_property(sink);
    if (!p2) throw std::runtime_error("No such bool property: " + sink.str());

    connections_.emplace_back(std::make_unique<bool_connection>(p1, p2));
    return dynamic_cast<bool_connection*>(connections_.back().get());
}

string_connection* simulation::make_string_connection(const variable_identifier& source, const variable_identifier& sink)
{
    const auto p1 = get_string_property(source);
    if (!p1) throw std::runtime_error("No such string property: " + source.str());
    const auto p2 = get_string_property(sink);
    if (!p2) throw std::runtime_error("No such string property: " + sink.str());

    connections_.emplace_back(std::make_unique<string_connection>(p1, p2));
    return dynamic_cast<string_connection*>(connections_.back().get());
}

property_t<double>* simulation::get_real_property(const variable_identifier& identifier) const
{
    for (auto& instance : instances_) {
        if (instance->instanceName() == identifier.instanceName) {
            const auto p = instance->get_properties().get_real_property(identifier.variableName);
            if (p) return p;
        }
    }
    return nullptr;
}

property_t<int>* simulation::get_int_property(const variable_identifier& identifier) const
{
    for (auto& instance : instances_) {
        if (instance->instanceName() == identifier.instanceName) {
            const auto p = instance->get_properties().get_int_property(identifier.variableName);
            if (p) return p;
        }
    }
    return nullptr;
}

property_t<std::string>* simulation::get_string_property(const variable_identifier& identifier) const
{
    for (auto& instance : instances_) {
        if (instance->instanceName() == identifier.instanceName) {
            auto p = instance->get_properties().get_string_property(identifier.variableName);
            if (p) return p;
        }
    }
    return nullptr;
}

property_t<bool>* simulation::get_bool_property(const variable_identifier& identifier) const
{
    for (auto& instance : instances_) {
        if (instance->instanceName() == identifier.instanceName) {
            const auto p = instance->get_properties().get_bool_property(identifier.variableName);
            if (p) return p;
        }
    }
    return nullptr;
}

const std::vector<std::unique_ptr<model_instance>>& simulation::get_instances() const
{
    return instances_;
}

std::vector<variable_identifier> simulation::identifiers() const
{
    std::vector<variable_identifier> ids;
    for (const auto& instance : instances_) {
        for (const auto& p : instance->get_properties().get_property_names()) {
            ids.emplace_back(instance->instanceName(), p);
        }
    }
    return ids;
}
