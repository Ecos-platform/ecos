
#ifndef VICO_SIMULATION_LISTENER_HPP
#define VICO_SIMULATION_LISTENER_HPP

#include "vico/simulation.hpp"
#include "vico/util/uuid.hpp"

namespace vico {

struct simulation_listener {

    const std::string uuid = generate_uuid();

    virtual void pre_init() = 0;
    virtual void post_init() = 0;

    virtual void pre_step() = 0;
    virtual void post_step() = 0;

    virtual void post_terminate() = 0;

    virtual ~simulation_listener() = default;

};

}

#endif // VICO_SIMULATION_LISTENER_HPP
