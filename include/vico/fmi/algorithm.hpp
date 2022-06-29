
#ifndef VICO_ALGORITHM_HPP
#define VICO_ALGORITHM_HPP

#include <fmilibcpp/buffered_slave.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace vico
{

struct algorithm
{

    virtual void init(double startTime) = 0;

    virtual void step(double currentTime, double stepSize, std::function<void(fmilibcpp::slave*)> stepCallback) = 0;

    virtual void terminate() = 0;

    virtual ~algorithm() = default;

protected:
    std::vector<fmilibcpp::buffered_slave*> slaves_;

    virtual void slave_added(fmilibcpp::slave* slave) = 0;

    virtual void slave_removed(fmilibcpp::slave* slave) = 0;

private:
    void slave_added_internal(fmilibcpp::buffered_slave* instance)
    {
        slave_added(instance);
        slaves_.emplace_back(instance);
    }

    void slave_removed_internal(fmilibcpp::buffered_slave* instance)
    {
        slave_removed(instance);
        auto remove = std::remove(slaves_.begin(), slaves_.end(), instance);
        slaves_.erase(remove, slaves_.end());
    }

    friend class fmi_system;
};


} // namespace vico

#endif // VICO_ALGORITHM_HPP
