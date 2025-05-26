
#ifndef ECOS_SIMULATION_HPP
#define ECOS_SIMULATION_HPP

#include "ecos/algorithm/algorithm.hpp"
#include "ecos/connection.hpp"
#include "ecos/listeners/simulation_listener.hpp"
#include "ecos/model_instance.hpp"
#include "ecos/variable_identifier.hpp"

#include <filesystem>
#include <memory>
#include <vector>

namespace ecos
{

/* *
 * \brief Represents a co-simulation.
 *
 * This class manages the lifecycle of a simulation, including initialization, stepping through time,
 * and termination. It supports adding model instances, listeners, and connections between variables.
 */
class simulation
{

public:
    explicit simulation(std::unique_ptr<algorithm> algorithm);

    simulation(const simulation&) = delete;
    simulation(simulation&&) = delete;
    simulation& operator=(simulation&&) = delete;
    simulation& operator=(const simulation&) = delete;

    [[nodiscard]] double time() const;

    [[nodiscard]] unsigned long iterations() const;

    [[nodiscard]] bool initialized() const;

    [[nodiscard]] bool terminated() const;

    void init(const std::string& parameterSet)
    {
        init(std::nullopt, parameterSet);
    }

    // Initializes the simulation with an optional start time and parameter set.
    void init(std::optional<double> startTime = std::nullopt, const std::optional<std::string>& parameterSet = std::nullopt);

    // Steps the simulation by a specified number of steps.
    double step(unsigned int numStep = 1);

    // Steps the simulation until a specified time point.
    void step_until(double timePoint);

    // Steps the simulation for a specified duration.
    void step_for(double duration);

    // Terminates the simulation.
    void terminate();

    // Resets the simulation to its initial state.
    void reset();

    // Adds a model instance to the simulation.
    void add_slave(std::unique_ptr<model_instance> slave);

    void add_listener(const std::string& name, std::shared_ptr<simulation_listener> listener);

    void remove_listener(const std::string& name);

    [[nodiscard]] model_instance* get_instance(const std::string& name) const;

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

    void on_init(const std::function<void()>& f);

    void invoke_when(const std::function<bool()>& predicate, const std::function<void()>& action);

    void invoke_at(double timePoint, const std::function<void()>& f, const std::optional<double>& eps = 0);

    void load_scenario(const std::filesystem::path& config);

    ~simulation();

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace ecos

#endif // ECOS_SIMULATION_HPP
