
#ifndef ECOS_COMPONENT_HPP
#define ECOS_COMPONENT_HPP

#include "property.hpp"

#include <stdexcept>
#include <unordered_map>

namespace ecos
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

    virtual void reset()
    {
        throw std::runtime_error("Reset not implemented!");
    }

    bool apply_parameter_set(const std::string& name)
    {
        if (parameterSets_.count(name)) {

            auto& parameters = parameterSets_.at(name);
            for (const auto& [variableName, value] : parameters) {

                switch (value.index()) {
                    case 0: {
                        auto p = properties_.get_real_property(variableName);
                        if (!p) {
                            throw std::runtime_error("No variable named '" + variableName +
                                "' of type real registered for instance '" + instanceName + "'");
                        }
                        p->set_value(std::get<double>(value));
                        break;
                    }
                    case 1: {
                        auto p = properties_.get_int_property(variableName);
                        if (!p) {
                            throw std::runtime_error("No variable named '" + variableName +
                                "' of type int registered for instance '" + instanceName + "'");
                        }
                        p->set_value(std::get<int>(value));
                        break;
                    }
                    case 2: {
                        auto p = properties_.get_bool_property(variableName);
                        if (!p) {
                            throw std::runtime_error("No variable named '" + variableName +
                                "' of type bool registered for instance '" + instanceName + "'");
                        }
                        p->set_value(std::get<bool>(value));
                        break;
                    }
                    case 3: {
                        auto p = properties_.get_string_property(variableName);
                        if (!p) {
                            throw std::runtime_error("No variable named '" + variableName +
                                "' of type string registered for instance '" + instanceName + "'");
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

    void add_parameter_set(const std::string& name, const std::unordered_map<std::string, std::variant<double, int, bool, std::string>>& parameterSet)
    {
        parameterSets_.emplace(name, parameterSet);
    }

    void add_parameterset_entry(
        const std::string& parameterSetName,
        const std::string& variableName,
        const std::variant<double, int, bool, std::string>& value)
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
    std::unordered_map<std::string, std::unordered_map<std::string, std::variant<double, int, bool, std::string>>> parameterSets_;
};

} // namespace ecos

#endif // ECOS_COMPONENT_HPP
