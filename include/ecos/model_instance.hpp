
#ifndef ECOS_COMPONENT_HPP
#define ECOS_COMPONENT_HPP

#include "ecos/property.hpp"
#include "ecos/scalar.hpp"

#include <stdexcept>
#include <unordered_map>

namespace ecos
{

struct model_state
{
    virtual ~model_state() = default;
};

/* *
 * \brief Abstract base class for model instances.
 *
 * A model instance represents a specific instance of a model in a simulation.
 * It provides methods for initialization, stepping through time, and managing parameters.
 */
class model_instance
{

public:
    // Constructs a model instance with a given name and an optional step size hint.
    explicit model_instance(std::string instanceName, std::optional<double> stepSizeHint = std::nullopt)
        : instanceName_(std::move(instanceName))
        , stepSizeHint_(stepSizeHint)
    { }

    model_instance(const model_instance&) = delete;
    model_instance(const model_instance&&) = delete;

    [[nodiscard]] const std::string& instanceName() const
    {
        return instanceName_;
    }

    // Returns the step size hint for this model instance, if available.
    // This hint can be used by algorithms to optimize the stepping process (multi-variate stepping).
    [[nodiscard]] const std::optional<double>& stepSizeHint() const
    {
        return stepSizeHint_;
    }

    virtual void enter_initialization_mode(double start = 0) = 0;

    virtual void exit_initialization_mode() = 0;

    virtual void step(double currentTime, double stepSize) = 0;

    virtual void terminate() = 0;

    virtual void reset() = 0;

    bool apply_parameter_set(const std::string& name);

    void add_parameter_set(const std::string& name, const std::unordered_map<std::string, scalar_value>& parameterSet);

    void add_parameterset_entry(
        const std::string& parameterSetName,
        const std::string& variableName,
        const scalar_value& value);

    properties& get_properties()
    {
        return properties_;
    }

    [[nodiscard]] const properties& get_properties() const
    {
        return properties_;
    }

    [[nodiscard]] virtual bool can_get_and_set_state() const
    {
        return false;
    }

    virtual std::unique_ptr<model_state> get_state()
    {
        throw std::runtime_error("Not implemented!");
    }

    virtual void set_state(model_state&)
    {
        throw std::runtime_error("Not implemented!");
    }

    virtual ~model_instance() = default;

protected:
    properties properties_;

    std::string instanceName_;
    std::optional<double> stepSizeHint_;

    std::unordered_map<std::string, std::unordered_map<std::string, scalar_value>> parameterSets_;
};

} // namespace ecos

#endif // ECOS_COMPONENT_HPP
