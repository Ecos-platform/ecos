
#ifndef VICO_MODEL_DESCRIPTION_HPP
#define VICO_MODEL_DESCRIPTION_HPP

#include "vico/scalar_variable.hpp"

#include <algorithm>
#include <optional>

namespace vico
{

struct default_experiment
{
    std::optional<double> startTime;
    std::optional<double> stopTime;
    std::optional<double> tolerance;
    std::optional<double> stepSize;
};

struct model_description
{
    std::string guid;
    std::string modelName;
    std::string modelIdentifier;
    std::string description;

    model_variables modelVariables;
    default_experiment defaultExperiment;

    [[nodiscard]] std::optional<scalar_variable> get_by_name(const std::string& name) const
    {
        auto result = std::find_if(modelVariables.begin(), modelVariables.end(), [&name](const scalar_variable &s) {
            return s.name == name;
        });

        if (result != modelVariables.end()) {
            return *result;
        } else {
            return std::nullopt;
        }
    }

    [[nodiscard]] std::optional<scalar_variable> get_by_vr(const value_ref &vr) const
    {
        auto result = std::find_if(modelVariables.begin(), modelVariables.end(), [&vr](const scalar_variable &s) {
            return s.valueRef == vr;
        });

        if (result != modelVariables.end()) {
            return *result;
        } else {
            return std::nullopt;
        }
    }
};

} // namespace vico

#endif // VICO_MODEL_DESCRIPTION_HPP
