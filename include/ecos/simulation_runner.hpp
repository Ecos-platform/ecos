
#ifndef LIBECOS_SIMULATION_RUNNER_HPP
#define LIBECOS_SIMULATION_RUNNER_HPP

#include "simulation.hpp"

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <limits>
#include <thread>

namespace ecos
{

class simulation_runner
{

public:
    explicit simulation_runner(simulation& sim);

    double real_time_factor() const;

    double target_real_time_factor() const
    {
        return targetRtf_;
    }

    double wall_clock() const;

    simulation_runner& set_real_time_factor(double target);

    simulation_runner& set_callback(const std::optional<std::function<void()>>& callback);

    void start()
    {
        run_while([] { return true; });
    }

    std::future<void> run_while(std::function<bool()> predicate);

    void stop();

    void toggle_pause()
    {
        paused_ = !paused_;
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

    void run();
};

} // namespace ecos

#endif // LIBECOS_SIMULATION_RUNNER_HPP
