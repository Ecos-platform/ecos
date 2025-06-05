
#ifndef LIBECOS_SIMULATION_RUNNER_HPP
#define LIBECOS_SIMULATION_RUNNER_HPP

#include "ecos/simulation.hpp"

#include <atomic>
#include <functional>
#include <future>
#include <limits>
#include <optional>
#include <thread>

namespace ecos
{

/* *
 * \brief A class to run a simulation in a separate thread.
 *
 * This class manages the execution of a simulation, allowing it to run at a specified real-time factor,
 * and provides methods to start, stop, and pause the simulation.
 */
class simulation_runner
{

public:
    explicit simulation_runner(simulation& sim);

    simulation_runner(const simulation_runner&) = delete;
    simulation_runner(simulation_runner&&) = delete;
    simulation_runner& operator=(simulation_runner&&) = delete;
    simulation_runner& operator=(const simulation_runner&) = delete;

    // Return the actual real-time factor of the simulation.
    [[nodiscard]] double real_time_factor() const;

    /// Returns the target real-time factor for the simulation.
    [[nodiscard]] double target_real_time_factor() const;

    // Returns the wall clock time since the simulation started.
    [[nodiscard]] double wall_clock() const;

    simulation_runner& set_real_time_factor(double target);

    // Register a callback to be called after each simulation step.
    simulation_runner& set_callback(const std::optional<std::function<void()>>& callback);

    // Runs simulation while the predicate returns true.
    std::future<void> run_while(std::function<bool()> predicate);

    // Starts the simulation in a separate thread.
    void start();

    // Stops the simulation and waits for the thread to finish.
    void stop();

    // Togle on/off the pause state of the simulation.
    bool toggle_pause()
    {
        paused_ = !paused_;
        return paused_;
    }


private:
    simulation& sim_;

    std::thread t_;
    std::atomic<bool> stop_{false};
    std::atomic<bool> paused_{false};

    double wallClock_{};
    double targetRtf_{1.0};
    double rtf_ = -std::numeric_limits<double>::infinity();

    std::optional<std::function<void()>> callback_;
    std::function<bool()> predicate_;

    std::mutex m_;

    void run();
};

} // namespace ecos

#endif // LIBECOS_SIMULATION_RUNNER_HPP
