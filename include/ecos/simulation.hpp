
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

    // Adds a listener to the simulation with a specified name.
    void add_listener(const std::string& name, std::shared_ptr<simulation_listener> listener);

    // Removes a listener from the simulation by name.
    bool remove_listener(const std::string& name);

    // Retrieves a model instance by its name.
    [[nodiscard]] model_instance* get_instance(const std::string& name) const;

    // Creates a connection between two real-valued variables.
    real_connection* make_real_connection(const variable_identifier& source, const variable_identifier& sink);

    // Creates a connection between two integer-valued variables.
    int_connection* make_int_connection(const variable_identifier& source, const variable_identifier& sink);

    // Creates a connection between two boolean-valued variables.
    bool_connection* make_bool_connection(const variable_identifier& source, const variable_identifier& sink);

    // Creates a connection between two string-valued variables.
    string_connection* make_string_connection(const variable_identifier& source, const variable_identifier& sink);

    [[nodiscard]] property_t<double>* get_real_property(const variable_identifier& identifier) const;

    [[nodiscard]] property_t<int>* get_int_property(const variable_identifier& identifier) const;

    [[nodiscard]] property_t<std::string>* get_string_property(const variable_identifier& identifier) const;

    [[nodiscard]] property_t<bool>* get_bool_property(const variable_identifier& identifier) const;

    [[nodiscard]] const std::vector<std::unique_ptr<model_instance>>& get_instances() const;

    [[nodiscard]] std::vector<variable_identifier> identifiers() const;

    // Registers a function to be called when the simulation is initialized.
    void on_init(const std::function<void()>& f);

    // Register an action when a specified condition is met during simulation
    void invoke_when(const std::function<bool()>& predicate, const std::function<void()>& action);

    // Register an action to be invoked at a specific time point in the simulation.
    // eps is an optional tolerance for the time point.
    void invoke_at(double timePoint, const std::function<void()>& f, const std::optional<double>& eps = 0);

    void load_scenario(const std::filesystem::path& config);

    ~simulation();

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace ecos

#endif // ECOS_SIMULATION_HPP
