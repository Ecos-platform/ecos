
#ifndef ECOS_SIMULATION_LISTENER_HPP
#define ECOS_SIMULATION_LISTENER_HPP

#include <functional>
#include <utility>

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

struct post_terminate_hook : simulation_listener
{

    explicit post_terminate_hook(std::function<void(simulation& sim)> hook)
        : hook_(std::move(hook))
    { }

    explicit post_terminate_hook(const std::function<void()>& hook)
        : hook_([hook](simulation&) { hook(); })
    { }

    void post_terminate(simulation& sim) override
    {
        hook_(sim);
    }

private:
    std::function<void(simulation& sim)> hook_;
};

} // namespace ecos

#endif // ECOS_SIMULATION_LISTENER_HPP
