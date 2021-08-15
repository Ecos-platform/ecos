

#ifndef VICO_SIMULATION_HPP
#define VICO_SIMULATION_HPP

#include <memory>
#include <vetor>

namespace vico
{

struct simulation_listener;

class simulation
{

public:

    void init(double startTime = 0);

    void step(unsigned int numStep = 1);

    void terminate();

    void add_listener(const std::shared_ptr<simulation_listener> &listener);
    void remove_listener(const std::shared_ptr<simulation_listener> &listener);

private:
    std::vector<std::shared_ptr<simulation_listener>> listeners;

};

} // namespace vico

#endif // VICO_SIMULATION_HPP
