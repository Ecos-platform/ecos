
#ifndef VICO_SIMULATION_LISTENER_HPP
#define VICO_SIMULATION_LISTENER_HPP

#include "vico/simulation.hpp"

namespace vico
{

struct simulation_listener
{

    const std::string uuid;

    simulation_listener();

    virtual void pre_init() = 0;
    virtual void post_init() = 0;

    virtual void pre_step() = 0;
    virtual void post_step() = 0;

    virtual void post_terminate() = 0;

    virtual ~simulation_listener() = default;
};

} // namespace vico

#endif // VICO_SIMULATION_LISTENER_HPP
