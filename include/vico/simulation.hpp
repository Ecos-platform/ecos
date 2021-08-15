

#ifndef VICO_SIMULATION_HPP
#define VICO_SIMULATION_HPP

#include "vico/algorithm.hpp"

#include <memory>
#include <vector>

namespace vico
{

struct simulation_listener;

class simulation
{

public:

    explicit simulation(std::unique_ptr<algorithm> algorithm);

    void init(double startTime = 0);

    void step(unsigned int numStep = 1);

    void terminate();

    void add_listener(const std::shared_ptr<simulation_listener> &listener);

private:
    double currentTime = 0;
    std::unique_ptr<algorithm> algorithm_;
    std::vector<std::shared_ptr<simulation_listener>> listeners_;

};

} // namespace vico

#endif // VICO_SIMULATION_HPP
