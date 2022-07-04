

#ifndef VICO_SIMULATION_HPP
#define VICO_SIMULATION_HPP

#include "vico/algorithm/algorithm.hpp"
#include "vico/connection.hpp"
#include "vico/model_instance.hpp"
#include "vico/property.hpp"
#include "vico/variable_identifier.hpp"
#include "vico/listeners/simulation_listener.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace vico
{

//struct simulation_listener;

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

    real_connection* make_real_connection(const variable_identifier& source, const variable_identifier& sink)
    {
        auto p1 = get_real_property(source);
        if (!p1) throw std::runtime_error("No such real property: " + source.str());
        auto p2 = get_real_property(sink);
        if (!p2) throw std::runtime_error("No such real property: " + sink.str());

        connections_.emplace_back(std::make_unique<real_connection>(p1, p2));
        return dynamic_cast<real_connection*>(connections_.back().get());
    }

    int_connection* make_int_connection(const variable_identifier& source, const variable_identifier& sink)
    {
        auto p1 = get_int_property(source);
        if (!p1) throw std::runtime_error("No such int property: " + source.str());
        auto p2 = get_int_property(sink);
        if (!p2) throw std::runtime_error("No such int property: " + sink.str());

        connections_.emplace_back(std::make_unique<int_connection>(p1, p2));
        return dynamic_cast<int_connection*>(connections_.back().get());
    }

    bool_connection* make_bool_connection(const variable_identifier& source, const variable_identifier& sink)
    {
        auto p1 = get_bool_property(source);
        if (!p1) throw std::runtime_error("No such int property: " + source.str());
        auto p2 = get_bool_property(sink);
        if (!p2) throw std::runtime_error("No such int property: " + sink.str());

        connections_.emplace_back(std::make_unique<bool_connection>(p1, p2));
        return dynamic_cast<bool_connection*>(connections_.back().get());
    }

    string_connection* make_string_connection(const variable_identifier& source, const variable_identifier& sink)
    {
        auto p1 = get_string_property(source);
        if (!p1) throw std::runtime_error("No such int property: " + source.str());
        auto p2 = get_string_property(sink);
        if (!p2) throw std::runtime_error("No such int property: " + sink.str());

        connections_.emplace_back(std::make_unique<string_connection>(p1, p2));
        return dynamic_cast<string_connection*>(connections_.back().get());
    }

    void init(double startTime = 0);

    void step(unsigned int numStep = 1);

    void terminate();

    void add_slave(std::unique_ptr<model_instance> slave);

    void add_listener(std::unique_ptr<simulation_listener> listener);

    model_instance* get_instance(const std::string& name);

    property_t<double>* get_real_property(const std::string& identifier)
    {
        return get_real_property(variable_identifier{identifier});
    }

    property_t<double>* get_real_property(const variable_identifier& identifier)
    {
        for (auto& instance : instances_) {
            if (instance->instanceName == identifier.instanceName) {
                auto p = instance->get_properties().get_real_property(identifier.variableName);
                if (p) return p;
            }
        }
        return nullptr;
    }

    property_t<int>* get_int_property(const std::string& identifier)
    {
        return get_int_property(variable_identifier{identifier});
    }

    property_t<int>* get_int_property(const variable_identifier& identifier)
    {
        for (auto& instance : instances_) {
            if (instance->instanceName == identifier.instanceName) {
                auto p = instance->get_properties().get_int_property(identifier.variableName);
                if (p) return p;
            }
        }
        return nullptr;
    }

    property_t<std::string>* get_string_property(const std::string& identifier)
    {
        return get_string_property(variable_identifier{identifier});
    }

    property_t<std::string>* get_string_property(const variable_identifier& identifier)
    {
        for (auto& instance : instances_) {
            if (instance->instanceName == identifier.instanceName) {
                auto p = instance->get_properties().get_string_property(identifier.variableName);
                if (p) return p;
            }
        }
        return nullptr;
    }

    property_t<bool>* get_bool_property(const std::string& identifier)
    {
        return get_bool_property(variable_identifier{identifier});
    }

    property_t<bool>* get_bool_property(const variable_identifier& identifier)
    {
        for (auto& instance : instances_) {
            if (instance->instanceName == identifier.instanceName) {
                auto p = instance->get_properties().get_bool_property(identifier.variableName);
                if (p) return p;
            }
        }
        return nullptr;
    }


    [[nodiscard]] const std::vector<std::unique_ptr<model_instance>>& get_instances() const
    {
        return instances_;
    }

private:
    double currentTime{0};
    unsigned long num_iterations{0};

    bool initialized{false};

    std::unique_ptr<algorithm> algorithm_;
    std::vector<std::unique_ptr<model_instance>> instances_;
    std::vector<std::unique_ptr<connection>> connections_;
    std::vector<std::unique_ptr<simulation_listener>> listeners_;
};

} // namespace vico

#endif // VICO_SIMULATION_HPP
