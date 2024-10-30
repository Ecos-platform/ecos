
#ifndef ECOS_COMPONENT_HPP
#define ECOS_COMPONENT_HPP

#include "ecos/property.hpp"
#include "ecos/scalar.hpp"

#include <stdexcept>
#include <unordered_map>

namespace ecos
{

class model_instance
{

public:
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

    [[nodiscard]] const std::optional<double>& stepSizeHint() const
    {
        return stepSizeHint_;
    }

    virtual void enter_initialization_mode(double start = 0) = 0;

    virtual void exit_initialization_mode() = 0;

    virtual void step(double currentTime, double stepSize) = 0;

    virtual void terminate() = 0;

    virtual void reset()
    {
        throw std::runtime_error("Reset not implemented!");
    }

    bool apply_parameter_set(const std::string& name)
    {
        if (parameterSets_.contains(name)) {

            auto& parameters = parameterSets_.at(name);
            for (const auto& [variableName, value] : parameters) {

                switch (value.index()) {
                    case 0: {
                        auto p = properties_.get_real_property(variableName);
                        if (!p) {
                            throw std::runtime_error("No variable named '" + variableName +
                                "' of type real registered for instance '" + instanceName_ + "'");
                        }
                        p->set_value(std::get<double>(value));
                        break;
                    }
                    case 1: {
                        auto p = properties_.get_int_property(variableName);
                        if (!p) {
                            throw std::runtime_error("No variable named '" + variableName +
                                "' of type int registered for instance '" + instanceName_ + "'");
                        }
                        p->set_value(std::get<int>(value));
                        break;
                    }
                    case 2: {
                        auto p = properties_.get_bool_property(variableName);
                        if (!p) {
                            throw std::runtime_error("No variable named '" + variableName +
                                "' of type bool registered for instance '" + instanceName_ + "'");
                        }
                        p->set_value(std::get<bool>(value));
                        break;
                    }
                    case 3: {
                        auto p = properties_.get_string_property(variableName);
                        if (!p) {
                            throw std::runtime_error("No variable named '" + variableName +
                                "' of type string registered for instance '" + instanceName_ + "'");
                        }
                        p->set_value(std::get<std::string>(value));
                        break;
                    }
                }
            }
            return true;
        }
        return false;
    }

    void add_parameter_set(const std::string& name, const std::unordered_map<std::string, scalar_value>& parameterSet)
    {
        parameterSets_.emplace(name, parameterSet);
    }

    void add_parameterset_entry(
        const std::string& parameterSetName,
        const std::string& variableName,
        const scalar_value& value)
    {
        parameterSets_[parameterSetName][variableName] = value;
    }

    properties& get_properties()
    {
        return properties_;
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
