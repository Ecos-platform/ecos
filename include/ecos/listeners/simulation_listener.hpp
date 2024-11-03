
#ifndef ECOS_SIMULATION_LISTENER_HPP
#define ECOS_SIMULATION_LISTENER_HPP


namespace ecos
{

class simulation;

struct simulation_listener
{

    virtual void pre_init(simulation& sim);
    virtual void post_init(simulation& sim);

    virtual void pre_step(simulation& sim);
    virtual void post_step(simulation& sim);

    virtual void post_terminate(simulation& sim);

    virtual void on_reset();

    virtual ~simulation_listener() = default;
};

} // namespace ecos

#endif // ECOS_SIMULATION_LISTENER_HPP
