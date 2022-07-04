
#ifndef VICO_FIXED_STEP_ALGORITHM_HPP
#define VICO_FIXED_STEP_ALGORITHM_HPP

#include "algorithm.hpp"

#include <algorithm>
#include <execution>

namespace vico
{

struct fixed_step_algorithm : public algorithm
{

public:
    explicit fixed_step_algorithm(double stepSize, bool parallel = true)
        : stepSize_(stepSize)
        , parallel_(parallel)
    { }

    double step(double currentTime, std::vector<std::unique_ptr<model_instance>>& instances) override
    {

        auto f = [currentTime, this](auto& instance) {
            instance->applySets();
            instance->step(currentTime, stepSize_);
            instance->applyGets();
        };

        if (!parallel_) {
            std::for_each(instances.begin(), instances.end(), f);
        } else {
            std::for_each(std::execution::par, instances.begin(), instances.end(), f);
        }

        return currentTime + stepSize_;
    }

    ~fixed_step_algorithm() override = default;

private:
    bool parallel_;
    double stepSize_;
};


} // namespace vico

#endif // VICO_FIXED_STEP_ALGORITHM_HPP
