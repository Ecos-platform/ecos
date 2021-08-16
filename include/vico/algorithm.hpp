
#ifndef VICO_ALGORITHM_HPP
#define VICO_ALGORITHM_HPP

#include <vico/model_instance.hpp>

#include <memory>
#include <vector>

namespace vico
{

struct algorithm
{

    virtual void instance_added(model_instance* instance) = 0;

    virtual void instance_removed(model_instance *instance) = 0;

    virtual void init(double startTime) = 0;

    virtual void step(double currentTime, double stepSize) = 0;

    virtual ~algorithm() = default;

protected:
    std::vector<model_instance*> instances_;
};

struct fixed_step_algorithm : public algorithm
{

    void init(double startTime) override
    {
        for (auto instance : instances_) {
            instance->setup_experiment(startTime);
            instance->enter_initialization_mode();
        }

        for (auto instance : instances_) {

        }
    }

    void step(double currentTime, double stepSize) override
    {
    }

    ~fixed_step_algorithm() override = default;
};

} // namespace vico

#endif // VICO_ALGORITHM_HPP
