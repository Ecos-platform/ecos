
#ifndef ECOS_FMI_MODEL_DESCRIPTION_HPP
#define ECOS_FMI_MODEL_DESCRIPTION_HPP

#include "scalar_variable.hpp"

#include <algorithm>
#include <optional>
#include <vector>

namespace fmilibcpp
{

struct default_experiment
{
    double startTime{0};
    double stopTime{0};
    double tolerance{0};
    double stepSize{0};
};

struct model_description
{
    std::string guid;
    std::string author;
    std::string modelName;
    std::string modelIdentifier;
    std::string fmiVersion;
    std::string description;
    std::string generationTool;
    std::string generationDateAndTime;

    model_variables modelVariables;
    default_experiment defaultExperiment;

    [[nodiscard]] std::optional<scalar_variable> get_by_name(const std::string& name) const
    {
        auto result = std::find_if(modelVariables.begin(), modelVariables.end(), [&name](const scalar_variable& s) {
            return s.name == name;
        });

        if (result != modelVariables.end()) {
            return *result;
        } else {
            return std::nullopt;
        }
    }

    template<class T>
    [[nodiscard]] std::optional<scalar_variable> get_by_vr(value_ref vr) const
    {
        auto result = std::find_if(modelVariables.begin(), modelVariables.end(), [&vr](const scalar_variable& s) {
            if (std::is_same<T, int>() && s.is_integer()) {
                return s.vr == vr;
            } else if (std::is_same<T, double>() && s.is_real()) {
                return s.vr == vr;
            } else if (std::is_same<T, std::string>() && s.is_string()) {
                return s.vr == vr;
            } else if (std::is_same<T, bool>() && s.is_boolean()) {
                return s.vr == vr;
            } else {
                return false;
            }
        });

        if (result != modelVariables.end()) {
            return *result;
        } else {
            return std::nullopt;
        }
    }
};

} // namespace fmi

#endif // ECOS_FMI_MODEL_DESCRIPTION_HPP
