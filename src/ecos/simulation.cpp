
#include "ecos/simulation.hpp"

#include "ecos/listeners/simulation_listener.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/property.hpp"
#include "ecos/scenario/scenario.hpp"

#include <execution>
#include <pugixml.hpp>
#include <ranges>

using namespace ecos;

struct simulation::Impl
{
    double lastDelta_{};
    double currentTime_{0};
    bool initialized_{false};
    bool terminated_{false};
    unsigned long num_iterations_{0};

    scenario scenario_;
    std::unique_ptr<algorithm> algorithm_;
    std::vector<std::unique_ptr<model_instance>> instances_;
    std::vector<std::unique_ptr<connection>> connections_;
    std::unordered_map<std::string, std::shared_ptr<simulation_listener>> listeners_;

    simulation& sim_;

    explicit Impl(simulation& sim, std::unique_ptr<algorithm> algorithm)
        : algorithm_(std::move(algorithm))
        , sim_(sim)
    { }

    double step(unsigned int numStep)
    {
        if (!initialized_) {
            throw std::runtime_error("init() has not been invoked!");
        }

        double newT{};
        for (unsigned i = 0; i < numStep; ++i) {

            for (auto l = listeners_; const auto& listener : l | std::views::values) {
                listener->pre_step(sim_);
            }

            scenario_.apply(currentTime_);

            newT = algorithm_->step(currentTime_);

            for (const auto& c : connections_) {
                c->transferData();
            }

            std::for_each(std::execution::par, instances_.begin(), instances_.end(), [](auto& instance) {
                instance->get_properties().apply_sets();
                instance->get_properties().apply_gets();
            });

            lastDelta_ = newT - currentTime_;
            currentTime_ = newT;
            ++num_iterations_;

            for (auto l = listeners_; const auto& listener : l | std::views::values) {
                listener->post_step(sim_);
            }
        }

        return currentTime_;
    }

    void load_scenario(const std::filesystem::path& config)
    {

        if (!exists(config)) {
            throw std::runtime_error("No such file: " + absolute(config).string());
        }

        if (const auto ext = config.extension().string(); ext != ".xml") {
            throw std::runtime_error("Wrong config extension. Was " + ext + ", expected " + ".xml");
        }

        pugi::xml_document doc;
        if (pugi::xml_parse_result result = doc.load_file(config.c_str()); !result) {
            throw std::runtime_error("Unable to parse '" + absolute(config).string() + "': " + result.description());
        }

        size_t numActions{};
        const auto root = doc.child("ecos:Scenario");
        const auto epsAttr = root.attribute("eps");
        std::optional<double> eps;
        if (epsAttr) eps = epsAttr.as_double();
        for (const auto& action : root) {
            ++numActions;
            const auto t = action.attribute("t").as_double();

            const auto epsSubAttr = action.attribute("eps");
            std::optional<double> subEps = eps;
            if (epsSubAttr) subEps = epsSubAttr.as_double();

            for (const auto& variable : action) {

                const variable_identifier id = variable.attribute("id").as_string();

                pugi::xml_node var;
                if ((var = variable.child("ecos:real"))) {
                    auto p = sim_.get_real_property(id);
                    if (!p) {
                        log::warn("No variable with id: {}", id.str());
                        continue;
                    }
                    const double value = var.attribute("value").as_double();
                    sim_.invoke_at(
                        t, [p, value] {
                            p->set_value(value);
                        },
                        subEps);
                } else if ((var = variable.child("ecos:integer"))) {
                    auto p = sim_.get_int_property(id);
                    if (!p) {
                        log::warn("No variable with id: {}", id.str());
                        continue;
                    }
                    const int value = var.attribute("value").as_int();
                    sim_.invoke_at(
                        t, [p, value] {
                            p->set_value(value);
                        },
                        subEps);
                } else if ((var = variable.child("ecos:boolean"))) {
                    auto p = sim_.get_bool_property(id);
                    if (!p) {
                        log::warn("No variable with id: {}", id.str());
                        continue;
                    }
                    const bool value = var.attribute("value").as_bool();
                    sim_.invoke_at(
                        t, [p, value] {
                            p->set_value(value);
                        },
                        subEps);
                } else if ((var = variable.child("ecos:string"))) {
                    auto p = sim_.get_string_property(id);
                    if (!p) {
                        log::warn("No variable with id: {}", id.str());
                        continue;
                    }
                    const std::string value = var.attribute("value").as_string();
                    sim_.invoke_at(
                        t, [p, value] {
                            p->set_value(value);
                        },
                        subEps);
                } else {
                    throw std::runtime_error("Assertion error");
                }
            }
        }

        log::debug("Applied scenario {} with {} actions to simulation", config.string(), numActions);
    }
};


simulation::simulation(std::unique_ptr<algorithm> algorithm)
    : pimpl_(std::make_unique<Impl>(*this, std::move(algorithm)))
{ }

double simulation::time() const
{
    return pimpl_->currentTime_;
}

unsigned long simulation::iterations() const
{
    return pimpl_->num_iterations_;
}

bool simulation::initialized() const
{
    return pimpl_->initialized_;
}

bool simulation::terminated() const
{
    return pimpl_->terminated_;
}

void simulation::init(std::optional<double> startTime, const std::optional<std::string>& parameterSet)
{
    if (!pimpl_->initialized_) {

        pimpl_->initialized_ = true;
        log::debug("Initializing simulation..");

        for (auto l = pimpl_->listeners_; const auto& listener : l | std::views::values) {
            listener->pre_init(*this);
        }

        int parameterSetAppliedCount = 0;
        for (const auto& instance : pimpl_->instances_) {
            double start = startTime.value_or(0);
            if (start < 0) {
                throw std::runtime_error("Explicitly defined startTime must be greater than 0!");
            }

            instance->enter_initialization_mode(start);
            if (parameterSet) {
                if (instance->apply_parameter_set(*parameterSet)) {
                    ++parameterSetAppliedCount;
                }
            }
        }
        if (parameterSet) {
            log::debug("Parameterset '{}' applied to {} instances", *parameterSet, parameterSetAppliedCount);
        }

        pimpl_->scenario_.runInitActions();

        for (unsigned i = 0; i < pimpl_->instances_.size(); ++i) {
            for (const auto& instance : pimpl_->instances_) {
                instance->get_properties().apply_sets();
                instance->get_properties().apply_gets();
            }
            for (const auto& c : pimpl_->connections_) {
                c->transferData();
            }
        }

        for (const auto& instance : pimpl_->instances_) {
            instance->exit_initialization_mode();
            instance->get_properties().apply_gets();
        }

        for (const auto& c : pimpl_->connections_) {
            c->transferData();
        }

        for (const auto& instance : pimpl_->instances_) {
            instance->get_properties().apply_sets();
            instance->get_properties().apply_gets();
        }

        for (auto l = pimpl_->listeners_; const auto& listener : pimpl_->listeners_ | std::views::values) {
            listener->post_init(*this);
        }

        log::debug("Initialized.");
    }
}

double simulation::step(unsigned int numStep)
{
    return pimpl_->step(numStep);
}

void simulation::step_until(double t)
{
    if (t <= pimpl_->currentTime_) {
        log::warn("Input time {} is not greater than the current simulation time {}. Simulation will not progress.", t, pimpl_->currentTime_);
    } else {
        double newT = (pimpl_->currentTime_ + pimpl_->lastDelta_);
        while (newT < t) {
            step();
            newT = pimpl_->currentTime_ + pimpl_->lastDelta_;
        }
    }
}

void simulation::step_for(double t)
{
    const double newT = pimpl_->currentTime_ + t;
    step_until(newT);
}

void simulation::terminate()
{
    if (!pimpl_->terminated_) {
        pimpl_->terminated_ = true;

        log::debug("Terminating simulation..");

        for (const auto& instance : pimpl_->instances_) {
            instance->terminate();
        }

        for (auto l = pimpl_->listeners_; const auto& listener : l | std::views::values) {
            listener->post_terminate(*this);
        }

        log::debug("Terminated.");
    }
}

void simulation::reset()
{
    log::debug("Resetting simulation at t={}", time());
    for (const auto& instance : pimpl_->instances_) {
        instance->reset();
    }
    pimpl_->scenario_.reset();
    pimpl_->currentTime_ = 0;
    pimpl_->num_iterations_ = 0;
    pimpl_->initialized_ = false;
}

void simulation::add_listener(const std::string& name, std::shared_ptr<simulation_listener> listener)
{
    if (pimpl_->listeners_.contains(name)) {
        log::warn("A listener named {} already exists..", name);
    } else {
        pimpl_->listeners_[name] = std::move(listener);
    }
}

void simulation::remove_listener(const std::string& name)
{
    log::debug("Removing listener named {}", name);
    pimpl_->listeners_.erase(name);
}

model_instance* simulation::get_instance(const std::string& name) const
{
    for (auto& instance : pimpl_->instances_) {
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

    pimpl_->instances_.emplace_back(std::move(instance));
    pimpl_->algorithm_->model_instance_added(pimpl_->instances_.back().get());
}

real_connection* simulation::make_real_connection(const variable_identifier& source, const variable_identifier& sink)
{
    const auto p1 = get_real_property(source);
    if (!p1) throw std::runtime_error("No such real property: " + source.str());
    const auto p2 = get_real_property(sink);
    if (!p2) throw std::runtime_error("No such real property: " + sink.str());

    pimpl_->connections_.emplace_back(std::make_unique<real_connection>(p1, p2));
    return dynamic_cast<real_connection*>(pimpl_->connections_.back().get());
}

int_connection* simulation::make_int_connection(const variable_identifier& source, const variable_identifier& sink)
{
    const auto p1 = get_int_property(source);
    if (!p1) throw std::runtime_error("No such int property: " + source.str());
    const auto p2 = get_int_property(sink);
    if (!p2) throw std::runtime_error("No such int property: " + sink.str());

    pimpl_->connections_.emplace_back(std::make_unique<int_connection>(p1, p2));
    return dynamic_cast<int_connection*>(pimpl_->connections_.back().get());
}

bool_connection* simulation::make_bool_connection(const variable_identifier& source, const variable_identifier& sink)
{
    const auto p1 = get_bool_property(source);
    if (!p1) throw std::runtime_error("No such bool property: " + source.str());
    const auto p2 = get_bool_property(sink);
    if (!p2) throw std::runtime_error("No such bool property: " + sink.str());

    pimpl_->connections_.emplace_back(std::make_unique<bool_connection>(p1, p2));
    return dynamic_cast<bool_connection*>(pimpl_->connections_.back().get());
}

string_connection* simulation::make_string_connection(const variable_identifier& source, const variable_identifier& sink)
{
    const auto p1 = get_string_property(source);
    if (!p1) throw std::runtime_error("No such string property: " + source.str());
    const auto p2 = get_string_property(sink);
    if (!p2) throw std::runtime_error("No such string property: " + sink.str());

    pimpl_->connections_.emplace_back(std::make_unique<string_connection>(p1, p2));
    return dynamic_cast<string_connection*>(pimpl_->connections_.back().get());
}

property_t<double>* simulation::get_real_property(const variable_identifier& identifier) const
{
    for (auto& instance : pimpl_->instances_) {
        if (instance->instanceName() == identifier.instanceName) {
            const auto p = instance->get_properties().get_real_property(identifier.variableName);
            if (p) return p;
        }
    }
    return nullptr;
}

property_t<int>* simulation::get_int_property(const variable_identifier& identifier) const
{
    for (auto& instance : pimpl_->instances_) {
        if (instance->instanceName() == identifier.instanceName) {
            const auto p = instance->get_properties().get_int_property(identifier.variableName);
            if (p) return p;
        }
    }
    return nullptr;
}

property_t<std::string>* simulation::get_string_property(const variable_identifier& identifier) const
{
    for (auto& instance : pimpl_->instances_) {
        if (instance->instanceName() == identifier.instanceName) {
            auto p = instance->get_properties().get_string_property(identifier.variableName);
            if (p) return p;
        }
    }
    return nullptr;
}

property_t<bool>* simulation::get_bool_property(const variable_identifier& identifier) const
{
    for (auto& instance : pimpl_->instances_) {
        if (instance->instanceName() == identifier.instanceName) {
            const auto p = instance->get_properties().get_bool_property(identifier.variableName);
            if (p) return p;
        }
    }
    return nullptr;
}

const std::vector<std::unique_ptr<model_instance>>& simulation::get_instances() const
{
    return pimpl_->instances_;
}

std::vector<variable_identifier> simulation::identifiers() const
{
    std::vector<variable_identifier> ids;
    for (const auto& instance : pimpl_->instances_) {
        for (const auto& p : instance->get_properties().get_property_names()) {
            ids.emplace_back(instance->instanceName(), p);
        }
    }
    return ids;
}

void simulation::on_init(const std::function<void()>& f)
{
    pimpl_->scenario_.on_init(f);
}

void simulation::invoke_when(const std::function<bool()>& predicate, const std::function<void()>& action)
{
    pimpl_->scenario_.invoke_when(predicate_action{predicate, action});
}

void simulation::invoke_at(double timePoint, const std::function<void()>& f, const std::optional<double>& eps)
{
    pimpl_->scenario_.invoke_at(timed_action(timePoint, f, eps));
}

void simulation::load_scenario(const std::filesystem::path& config)
{
    pimpl_->load_scenario(config);
}

simulation::~simulation() = default;
