
#ifndef VICO_SIMULATION_LISTENER_HPP
#define VICO_SIMULATION_LISTENER_HPP

#include "vico/simulation.hpp"
#include "vico/util/uuid.hpp"

#include <functional>

namespace vico {

struct simulation_listener {

    const std::string uuid = generate_uuid();

    virtual void pre_init(const std::function<void()> &f) = {};
    virtual void post_init(const std::function<void()> &f) = {};

    virtual void pre_step(const std::function<void()> &f) = {};
    virtual void post_step(const std::function<void()> &f) = {};

    virtual void post_terminate(const std::function<void()> &f) = {};

    virtual ~simulation_listener() = default;

};

}

#endif // VICO_SIMULATION_LISTENER_HPP
