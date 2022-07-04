

#ifndef VICO_SIMULATION_HPP
#define VICO_SIMULATION_HPP

#include "vico/connection.hpp"
#include "vico/fmi/algorithm.hpp"
#include "vico/model_instance.hpp"
#include "vico/property.hpp"
#include "vico/structure/variable_identifier.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace vico
{

struct simulation_listener;

class simulation
{

public:
    explicit simulation(std::unique_ptr<algorithm> algorithm);

    [[nodiscard]] double time() const
    {
        return currentTime;
    }

    [[nodiscard]] unsigned long iterations() const
    {
        return num_iterations;
    }

    template<class T>
    connection_t<T>* add_connection(const std::string& source, const std::string& sink)
    {
        return add_connection<T>(variable_identifier(source), variable_identifier(sink));
    }

    template<class T>
    connection_t<T>* add_connection(const variable_identifier& source, const variable_identifier& sink)
    {
        auto p1 = get_property<T>(source);
        if (!p1) throw std::runtime_error("No such property: " + source.str());
        auto p2 = get_property<T>(sink);
        if (!p2) throw std::runtime_error("No such property: " + sink.str());

        connections_.emplace_back(std::make_unique<connection_t<T>>(p1, p2));
        return static_cast<connection_t<T>*>(connections_.back().get());
    }

    void init(double startTime = 0);

    void step(unsigned int numStep = 1);

    void terminate();

    void add_slave(std::unique_ptr<model_instance> slave);

    void add_listener(const std::shared_ptr<simulation_listener>& listener);

    model_instance* get_instance(const std::string& name);

    //    void get_property_names(std::vector<std::string>& list) const
    //    {
    //        for (const auto& [name, _] : properties_) {
    //            list.emplace_back(name);
    //        }
    //    }

    template<class T>
    property_t<T>* get_property(const std::string& identifier)
    {
        return get_property<T>(variable_identifier(identifier));
    }

    template<class T>
    property_t<T>* get_property(const variable_identifier& identifier)
    {
        for (auto& instance : instances_) {
            if (instance->instanceName == identifier.instanceName) {
                auto p = instance->getProperty<T>(identifier.variableName);
                if (p) return p;
            }
        }
        return nullptr;
    }
    //
    //    template<class T>
    //    property_t<T>* get_property(const std::string& identifier)
    //    {
    //        if (properties_.count(identifier)) {
    //            return static_cast<property_t<T>*>(properties_.at(identifier).get());
    //        } else {
    //            return nullptr;
    //        }
    //    }

    //    template<class Base>
    //    vico::system* get_system()
    //    {
    //        for (auto& system : this->systems_) {
    //            if (dynamic_cast<Base*>(system.get())) {
    //                return system.get();
    //            }
    //        }
    //        return nullptr;
    //    }
    //
    //
    //    property* get_property(const std::string& identifier);
    //
    //    template<class T>
    //    property_t<T>* get_property(const std::string& identifier)
    //    {
    //
    //        for (const auto& system : systems_) {
    //            auto get = system->get_property<T>(identifier);
    //            if (get) return get;
    //        }
    //
    //        return nullptr;
    //    }
    //
    //    std::vector<std::string> get_property_names()
    //    {
    //        std::vector<std::string> list;
    //        for (const auto& system : systems_) {
    //            system->get_property_names(list);
    //        }
    //
    //        return list;
    //    }

private:
    double currentTime{0};
    unsigned long num_iterations{0};

    bool initialized{false};


    std::unique_ptr<algorithm> algorithm_;
    std::vector<std::unique_ptr<model_instance>> instances_;
    //    std::unordered_map<std::string, std::shared_ptr<property>> properties_;
        std::vector<std::unique_ptr<connection>> connections_;
    std::vector<std::shared_ptr<simulation_listener>> listeners_;

    //    void updateConnections();
};

} // namespace vico

#endif // VICO_SIMULATION_HPP
