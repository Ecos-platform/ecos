

#ifndef VICO_SIMULATION_HPP
#define VICO_SIMULATION_HPP

#include <memory>

namespace vico
{

struct simulation_listener;

class simulation
{

public:

    void step(unsigned int numStep = 1);

    void terminate();

    void add_listener(const std::shared_ptr<simulation_listener> &listener);
    void remove_listener(cosnt std::shared_ptr<simulation_listener> &listener);

private:
    struct pimpl;
    std::unique_ptr<pimpl> pimpl_;
};

} // namespace vico

#endif // VICO_SIMULATION_HPP
