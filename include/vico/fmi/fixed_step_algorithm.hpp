
#ifndef VICO_FIXED_STEP_ALGORITHM_HPP
#define VICO_FIXED_STEP_ALGORITHM_HPP

#include "algorithm.hpp"

namespace vico
{

struct fixed_step_algorithm : public algorithm
{

public:

    explicit fixed_step_algorithm(double stepSize): stepSize_(stepSize){}

    double step(double currentTime, std::function<void(fmilibcpp::slave*)> stepCallback) override
    {
        for (auto& slave : slaves_) {
            slave->transferCachedSets();
            slave->step(currentTime, stepSize_);
            slave->receiveCachedGets();
            stepCallback(slave);
        }
        return currentTime + stepSize_;
    }

    ~fixed_step_algorithm() override = default;


protected:
    void slave_added(fmilibcpp::slave* slave) override
    {
    }
    void slave_removed(fmilibcpp::slave* slave) override
    {
    }

private:
    double stepSize_;
};


} // namespace vico

#endif // VICO_FIXED_STEP_ALGORITHM_HPP
