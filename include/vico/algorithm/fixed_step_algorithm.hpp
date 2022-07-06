
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

    double step(double currentTime, std::vector<std::unique_ptr<model_instance>>& instances) override;

    ~fixed_step_algorithm() override = default;

private:
    bool parallel_;
    double stepSize_;
};


} // namespace vico

#endif // VICO_FIXED_STEP_ALGORITHM_HPP
