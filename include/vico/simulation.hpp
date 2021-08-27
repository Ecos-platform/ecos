

#ifndef VICO_SIMULATION_HPP
#define VICO_SIMULATION_HPP

#include "vico/connection.hpp"
#include "vico/fmi/algorithm.hpp"
#include "vico/system.hpp"

#include <memory>
#include <vector>

namespace vico
{

struct simulation_listener;

class simulation
{

public:
    explicit simulation(double baseStepSize);

    void add_system(std::unique_ptr<system> system);

    void add_connection(std::unique_ptr<connection> c);

    void init(double startTime = 0);

    void step(unsigned int numStep = 1);

    void terminate();

    void add_listener(const std::shared_ptr<simulation_listener>& listener);

    template<class T>
    std::optional<property_t<T>> get(const std::string& identifier)
    {

        for (const auto& system : systems_) {
            auto get = system->get<T>(identifier);
            if (get) return *get;
        }

        return std::nullopt;
    }

private:
    double baseStepSize;
    double currentTime = 0;

    std::vector<std::unique_ptr<system>> systems_;
    std::vector<std::unique_ptr<connection>> connections_;
    std::vector<std::shared_ptr<simulation_listener>> listeners_;

    void updateConnections();
};

} // namespace vico

#endif // VICO_SIMULATION_HPP
