
#include "ecos/model_instance.hpp"

#include "ecos/logger/logger.hpp"

using namespace ecos;


bool model_instance::apply_parameter_set(const std::string& name)
{
    if (parameterSets_.contains(name)) {

        const auto& parameters = parameterSets_.at(name);
        for (const auto& [variableName, value] : parameters) {

            switch (value.index()) {
                case 0: {
                    if (const auto p = properties_.get_real_property(variableName); !p) {
                        log::warn("No variable named '{}' of type real registered for instance '{}'", variableName, instanceName_);
                    } else {
                        p->set_value(std::get<double>(value));
                    }
                    break;
                }
                case 1: {
                    if (const auto p = properties_.get_int_property(variableName); p) {
                        p->set_value(std::get<int>(value));
                    } else {
                        if (const auto pr = properties_.get_real_property(variableName); pr) {
                            pr->set_value(std::get<int>(value));
                        } else {
                            log::warn("No variable named '{}' of type int registered for instance '{}'", variableName, instanceName_);
                        }
                    }
                    break;
                }
                case 2: {
                    if (const auto p = properties_.get_bool_property(variableName); !p) {
                        log::warn("No variable named '{}' of type bool registered for instance '{}'", variableName, instanceName_);
                    } else {
                        p->set_value(std::get<bool>(value));
                    }
                    break;
                }
                case 3: {
                    if (const auto p = properties_.get_string_property(variableName); !p) {
                        log::warn("No variable named '{}' of type string registered for instance '{}'", variableName, instanceName_);
                    } else {
                        p->set_value(std::get<std::string>(value));
                    }
                    break;
                }
            }
        }
        return true;
    }
    return false;
}

void model_instance::add_parameter_set(const std::string& name, const std::unordered_map<std::string, scalar_value>& parameterSet)
{
    parameterSets_.emplace(name, parameterSet);
}

void model_instance::add_parameterset_entry(const std::string& parameterSetName, const std::string& variableName, const scalar_value& value)
{
    parameterSets_[parameterSetName][variableName] = value;
}