
#include "ecos/simulation_runner.hpp"

#include <iostream>

using namespace ecos;

namespace
{

inline double measure_time(const std::function<void()>& f)
{
    const auto start = std::chrono::steady_clock::now();
    f();
    const auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
}

} // namespace

simulation_runner::simulation_runner(simulation& sim)
    : sim_(sim)
{ }

double simulation_runner::real_time_factor() const
{
    return rtf_;
}

double simulation_runner::wall_clock() const
{
    return wallClock_;
}

simulation_runner& simulation_runner::set_real_time_factor(double target)
{
    targetRtf_ = target > 0 ? target : std::numeric_limits<double>::infinity();
    return *this;
}

simulation_runner& simulation_runner::set_callback(const std::optional<std::function<void()>>& callback)
{
    callback_ = callback;
    return *this;
}

std::future<void> simulation_runner::run_while(std::function<bool()> predicate)
{
    predicate_ = std::move(predicate);

    return std::async(std::launch::async, [this] {
        run();

        std::lock_guard<std::mutex> lck(m_);
        if (t_.joinable()) {
            t_.join();
        }
    });
}

void simulation_runner::run()
{
    t_ = std::thread([this] {
        while (!stop_) {

            if (paused_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            const double elapsed = measure_time([&] {
                if (!predicate_()) {
                    stop_ = true;
                } else {

                    if (rtf_ < targetRtf_) {
                        sim_.step();

                        if (callback_) {
                            (*callback_)();
                        }
                    } else {
                        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
                    }
                }
            });

            const double t = sim_.time();
            wallClock_ += elapsed;
            rtf_ = t / wallClock_;
        }
    });
}

void simulation_runner::stop()
{
    stop_ = true;
    std::lock_guard<std::mutex> lck(m_);
    if (t_.joinable()) {
        t_.join();
    }
}
