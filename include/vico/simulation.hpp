

#ifndef VICO_SIMULATION_HPP
#define VICO_SIMULATION_HPP

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

    [[nodiscard]] double time() const
    {
        return currentTime;
    }

    [[nodiscard]] unsigned long iterations() const
    {
        return num_iterations;
    }

    void add_system(std::unique_ptr<system> system);

    template<class T>
    connection_t<T> &add_connection(const std::string& source, const std::string& sink)
    {
        auto p1 = get_property<T>(source);
        if (!p1) throw std::runtime_error("No such property: " + source);
        auto p2 = get_property<T>(sink);
        if (!p2) throw std::runtime_error("No such property: " + sink);

        return p1->addSink(p2);
    }

    void init(double startTime = 0);

    void step(unsigned int numStep = 1);

    void terminate();

    void add_listener(const std::shared_ptr<simulation_listener>& listener);

    template<class Base>
    vico::system* get_system()
    {
        for (auto& system : this->systems_) {
            if (dynamic_cast<Base*>(system.get())) {
                return system.get();
            }
        }
        return nullptr;
    }


    property* get_property(const std::string& identifier);

    template<class T>
    property_t<T>* get_property(const std::string& identifier)
    {

        for (const auto& system : systems_) {
            auto get = system->get_property<T>(identifier);
            if (get) return get;
        }

        return nullptr;
    }

    std::vector<std::string> get_property_names()
    {
        std::vector<std::string> list;
        for (const auto& system : systems_) {
            system->get_property_names(list);
        }

        return list;
    }

private:
    double baseStepSize;
    double currentTime{0};
    unsigned long num_iterations{0};

    bool initialized{false};

    std::vector<std::unique_ptr<system>> systems_;
    //    std::vector<std::unique_ptr<connection>> connections_;
    std::vector<std::shared_ptr<simulation_listener>> listeners_;

    //    void updateConnections();
};

} // namespace vico

#endif // VICO_SIMULATION_HPP
