
#ifndef VICO_FIXED_STEP_ALGORITHM_HPP
#define VICO_FIXED_STEP_ALGORITHM_HPP

#include "algorithm.hpp"

namespace vico {

struct fixed_step_algorithm : public algorithm
{

public:
    void init(double startTime) override
    {
        for (auto& slave : slaves_) {
            slave->setup_experiment(startTime);
            slave->enter_initialization_mode();
        }

        for (auto& slave : slaves_) {
            slave->transferCachedSets();
            slave->receiveCachedGets();
        }

        for (auto& slave : slaves_) {
            slave->exit_initialization_mode();
            slave->receiveCachedGets();
        }
    }

    void step(double currentTime, double stepSize, std::function<void(fmilibcpp::slave*)> stepCallback) override
    {
        for (auto& slave : slaves_) {
            slave->transferCachedSets();
            slave->step(currentTime, stepSize);
            slave->receiveCachedGets();
            stepCallback(slave);
        }
    }

    void terminate() override
    {
        for (auto& slave : slaves_) {
            slave->terminate();
        }
    }

    ~fixed_step_algorithm() override = default;


protected:
    void slave_added(fmilibcpp::slave* slave) override
    {
    }
    void slave_removed(fmilibcpp::slave* slave) override
    {
    }
};

}

#endif // VICO_FIXED_STEP_ALGORITHM_HPP
