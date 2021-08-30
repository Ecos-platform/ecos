

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

    [[nodiscard]] double time() const {
        return currentTime;
    }

    void add_system(std::unique_ptr<system> system);

//    void add_connection(std::unique_ptr<connection> c);

    template<class T>
    void add_connection(const std::string &source, const std::string &sink, std::optional<std::function<void(const T&)>> modifier = std::nullopt) {
        auto p1 = get_property<T>(source);
        if (!p1) throw std::runtime_error("No such property: " + source);
        auto p2 = get_property<T>(sink);
        if (!p2) throw std::runtime_error("No such property: " + sink);

//        if (p1->index() != p2->index() ) throw std::runtime_error("Type mismatch!");

//        if (std::holds_alternative<int_property>(*p1)) {
//            auto ip1 = std::get<int_property>(*p1);
//            auto ip2 =  std::get<int_property>(*p2);
            p1->addSink(p2);
//        } else if (std::holds_alternative<real_property>(*p1)) {
//            auto ip1 = std::get<real_property>(*p1);
//            auto ip2 =  std::get<real_property>(*p2);
//            ip1->addSink(ip2);
//        } else if (std::holds_alternative<string_property>(*p1)) {
//            auto ip1 = std::get<string_property>(*p1);
//            auto ip2 =  std::get<string_property>(*p2);
//            ip1->addSink(ip2);
//        } else if (std::holds_alternative<bool_property>(*p1)) {
//            auto ip1 = std::get<bool_property>(*p1);
//            auto ip2 =  std::get<bool_property>(*p2);
//            ip1->addSink(ip2);
//        } else {
//            throw std::runtime_error("Assertion error!");
//        }
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


    property *get_property(const std::string& identifier)
    {

        for (const auto& system : systems_) {
            auto get = system->get_property(identifier);
            if (get) return get;
        }

        return nullptr;
    }

    template<class T>
    property_t<T> *get_property(const std::string& identifier)
    {

        for (const auto& system : systems_) {
            auto get = system->get_property<T>(identifier);
            if (get) return get;
        }

        return nullptr;
    }

private:
    double baseStepSize;
    double currentTime = 0;

    bool initialized{false};

    std::vector<std::unique_ptr<system>> systems_;
//    std::vector<std::unique_ptr<connection>> connections_;
    std::vector<std::shared_ptr<simulation_listener>> listeners_;

//    void updateConnections();
};

} // namespace vico

#endif // VICO_SIMULATION_HPP
