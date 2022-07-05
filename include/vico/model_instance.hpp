
#ifndef VICO_COMPONENT_HPP
#define VICO_COMPONENT_HPP

#include "property.hpp"

#include <unordered_map>
#include <stdexcept>

namespace vico
{

class model_instance
{

public:
    const std::string instanceName;

    explicit model_instance(std::string instanceName)
        : instanceName(std::move(instanceName))
    { }

    model_instance(const model_instance&) = delete;
    model_instance(const model_instance&&) = delete;

    virtual void setup_experiment(double start = 0) = 0;

    virtual void enter_initialization_mode() = 0;

    virtual void exit_initialization_mode() = 0;

    virtual void step(double currentTime, double stepSize) = 0;

    virtual void terminate() = 0;

    virtual void reset() {
        throw std::runtime_error("Reset not implemented!");
    }

    properties& get_properties()
    {
        return properties_;
    }

    virtual ~model_instance() = default;

protected:
    properties properties_;
};

} // namespace vico

#endif // VICO_COMPONENT_HPP
