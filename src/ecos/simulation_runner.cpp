
#include "ecos/simulation_runner.hpp"

using namespace ecos;

namespace
{

inline double measure_time(std::function<void()> f)
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

void simulation_runner::set_callback(const std::optional<std::function<void()>>& callback)
{
    callback_ = callback;
}

std::future<void> simulation_runner::run_while(std::function<bool()> predicate)
{
    predicate_ = std::move(predicate);

    return std::async(std::launch::async, [this]{
        run();
        t_.join();
    });
}

void simulation_runner::run()
{
    t_ = std::thread([this] {
        while (!stop_) {

            if (paused_) continue;

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
                        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
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
    t_.join();
}
