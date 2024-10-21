
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

class simulation_runner
{

public:
    explicit simulation_runner(simulation& sim);

    [[nodiscard]] double real_time_factor() const;

    [[nodiscard]] double target_real_time_factor() const;

    [[nodiscard]] double wall_clock() const;

    simulation_runner& set_real_time_factor(double target);

    simulation_runner& set_callback(const std::optional<std::function<void()>>& callback);

    std::future<void> run_while(std::function<bool()> predicate);

    void start();

    void stop();

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
