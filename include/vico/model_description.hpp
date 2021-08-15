
#ifndef VICO_MODEL_DESCRIPTION_HPP
#define VICO_MODEL_DESCRIPTION_HPP

#include "vico/scalar_variable.hpp"

namespace vico
{

struct default_experiment
{
    double startTime;
    double stopTime;
    double tolerance;
    double stepSize;
};

struct model_description
{
    std::string guid;
    std::string modelName;
    std::string modelIdentifier;
    std::string description;

    model_variables modelVariables;
    default_experiment defaultExperiment;
};

} // namespace vico

#endif // VICO_MODEL_DESCRIPTION_HPP
