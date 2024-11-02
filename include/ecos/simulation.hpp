
#ifndef ECOS_SIMULATION_HPP
#define ECOS_SIMULATION_HPP

#include "ecos/algorithm/algorithm.hpp"
#include "ecos/connection.hpp"
#include "ecos/listeners/simulation_listener.hpp"
#include "ecos/model_instance.hpp"
#include "ecos/property.hpp"
#include "ecos/scenario/scenario.hpp"
#include "ecos/variable_identifier.hpp"

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

namespace ecos
{

class simulation
{

public:
    explicit simulation(std::unique_ptr<algorithm> algorithm);

    simulation(const simulation&) = delete;
    simulation(simulation&&) = delete;
    simulation& operator=(simulation&&) = delete;
    simulation& operator=(const simulation&) = delete;

    [[nodiscard]] double time() const
    {
        return currentTime_;
    }

    [[nodiscard]] unsigned long iterations() const
    {
        return num_iterations_;
    }

    [[nodiscard]] bool initialized() const
    {
        return initialized_;
    }

    [[nodiscard]] bool terminated() const
    {
        return terminated_;
    }

    void init(const std::string& parameterSet)
    {
        init(std::nullopt, parameterSet);
    }

    void init(std::optional<double> startTime = std::nullopt, const std::optional<std::string>& parameterSet = std::nullopt);

    double step(unsigned int numStep = 1);

    void step_until(double t);

    void step_for(double t);

    void terminate();

    void reset();

    void add_slave(std::unique_ptr<model_instance> slave);

    void add_listener(const std::string& name, std::shared_ptr<simulation_listener> listener);

    void remove_listener(const std::string& name);

    model_instance* get_instance(const std::string& name) const;

    real_connection* make_real_connection(const variable_identifier& source, const variable_identifier& sink);

    int_connection* make_int_connection(const variable_identifier& source, const variable_identifier& sink);

    bool_connection* make_bool_connection(const variable_identifier& source, const variable_identifier& sink);

    string_connection* make_string_connection(const variable_identifier& source, const variable_identifier& sink);

    [[nodiscard]] property_t<double>* get_real_property(const variable_identifier& identifier) const;

    [[nodiscard]] property_t<int>* get_int_property(const variable_identifier& identifier) const;

    [[nodiscard]] property_t<std::string>* get_string_property(const variable_identifier& identifier) const;

    [[nodiscard]] property_t<bool>* get_bool_property(const variable_identifier& identifier) const;

    [[nodiscard]] const std::vector<std::unique_ptr<model_instance>>& get_instances() const;

    [[nodiscard]] std::vector<variable_identifier> identifiers() const;

    void on_init(const std::function<void()>& f)
    {
        scenario_.on_init(f);
    }

    void invoke_when(const std::function<bool()>& predicate, const std::function<void()>& action)
    {
        scenario_.invoke_when(predicate_action{predicate, action});
    }

    void invoke_at(double timePoint, const std::function<void()>& f, const std::optional<double>& eps = 0)
    {
        scenario_.invoke_at(timed_action(timePoint, f, eps));
    }

private:
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
};

} // namespace ecos

#endif // ECOS_SIMULATION_HPP
