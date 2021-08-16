
#ifndef VICO_ALGORITHM_HPP
#define VICO_ALGORITHM_HPP

#include <vico/model_instance.hpp>

#include <memory>
#include <vector>

namespace vico
{

struct algorithm
{

    virtual void init() = 0;

    virtual void step(double currentTime, double stepSize) = 0;

    virtual ~algorithm() = default;
};

struct fixed_step_algorithm : public algorithm
{

    void init() override
    {
    }

    void step(double currentTime, double stepSize) override
    {
    }

    ~fixed_step_algorithm() override = default;
};

} // namespace vico

#endif // VICO_ALGORITHM_HPP
