
#ifndef VICO_SIMULATION_LISTENER_HPP
#define VICO_SIMULATION_LISTENER_HPP


namespace vico
{

class simulation;

struct simulation_listener
{

    virtual void pre_init(simulation& sim);
    virtual void post_init(simulation& sim);

    virtual void pre_step(simulation& sim);
    virtual void post_step(simulation& sim);

    virtual void post_terminate(simulation& sim);

    virtual ~simulation_listener() = default;
};

} // namespace vico

#endif // VICO_SIMULATION_LISTENER_HPP
