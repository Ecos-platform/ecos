
#ifndef VICO_ALGORITHM_HPP
#define VICO_ALGORITHM_HPP

#include <fmilibcpp/slave.hpp>
#include <memory>
#include <vector>
#include <functional>

namespace vico
{

struct algorithm
{

    virtual void init(double startTime) = 0;

    virtual void step(double currentTime, double stepSize, std::function<void(fmilibcpp::slave*)> stepCallback) = 0;

    virtual void terminate() = 0;

    virtual ~algorithm() = default;

protected:
    std::vector<fmilibcpp::slave*> slaves_;

    virtual void slave_added(fmilibcpp::slave* slave) = 0;

    virtual void slave_removed(fmilibcpp::slave* slave) = 0;

private:
    void slave_added_internal(fmilibcpp::slave* instance)
    {
        slave_added(instance);
        slaves_.emplace_back(instance);
    }

    void slave_removed_internal(fmilibcpp::slave* instance)
    {
        slave_removed(instance);
        auto remove = std::remove(slaves_.begin(), slaves_.end(), instance);
        slaves_.erase(remove, slaves_.end());
    }

    friend class fmi_system;
};

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
            //            instance->transferCachedSets();
            //            instance->retreiveCachedGets();
        }

        for (auto& slave : slaves_) {
            slave->exit_initialization_mode();

            //            instance->retreiveCachedGets();
        }
    }

    void step(double currentTime, double stepSize, std::function<void(fmilibcpp::slave*)> stepCallback) override
    {
        for (auto& slave : slaves_) {
            slave->step(currentTime, stepSize);
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

} // namespace vico

#endif // VICO_ALGORITHM_HPP
