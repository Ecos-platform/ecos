
#include "fmi2_model_description.hpp"

namespace
{

std::optional<std::string> fmi2CausalityToString(fmi2Causality causality)
{
    switch (causality) {
        case fmi2CausalityInput: return "Input";
        case fmi2CausalityOutput: return "Output";
        case fmi2CausalityParameter: return "Parameter";
        case fmi2CausalityCalculatedParameter: return "CalculatedParameter";
        case fmi2CausalityLocal: return "Local";
        case fmi2CausalityIndependent: return "Independent";
        default: return std::nullopt;
    }
}

std::optional<std::string> fmi2VariabilityToString(fmi2Variability variability)
{
    switch (variability) {
        case fmi2VariabilityFixed: return "Fixed";
        case fmi2VariabilityTunable: return "Tunable";
        case fmi2VariabilityConstant: return "Constant";
        case fmi2VariabilityDiscrete: return "Discrete";
        case fmi2VariabilityContinuous: return "Continuous";
        default: return std::nullopt;
    }
}

std::optional<fmilibcpp::scalar_variable> to_scalar_variable(fmi2VariableHandle* v)
{
    const auto type = fmi2_getVariableDataType(v);
    if (type == fmi2DataTypeEnumeration) {
        return std::nullopt;
    }

    fmilibcpp::scalar_variable var;
    var.vr = fmi2_getVariableValueReference(v);
    var.name = fmi2_getVariableName(v);
    //    var.description = fmi2_import_get_variable_description(v);
    var.causality = fmi2CausalityToString(fmi2_getVariableCausality(v));
    var.variability = fmi2VariabilityToString(fmi2_getVariableVariability(v));

    switch (type) {
        case fmi2DataTypeReal: {
            fmilibcpp::real_attributes r{};
            if (false) {
                r.start = fmi2_getVariableStartReal(v);
            }
            var.typeAttributes = r;
        } break;
        case fmi2DataTypeInteger: {
            fmilibcpp::integer_attributes i{};
            if (false) {
                i.start = fmi2_getVariableStartInteger(v);
            }
            var.typeAttributes = i;
        } break;
        case fmi2DataTypeBoolean: {
            fmilibcpp::boolean_attributes b{};
            if (false) {
                b.start = fmi2_getVariableStartBoolean(v);
            }
            var.typeAttributes = b;
        } break;
        case fmi2DataTypeString: {
            fmilibcpp::string_attributes s{};
            if (false) {
                s.start = fmi2_getVariableStartString(v);
            }
            var.typeAttributes = s;
        } break;
        case fmi2DataTypeEnumeration: break;
    }
    return var;
}

} // namespace

namespace fmilibcpp
{

model_description create_fmi2_model_description(fmiHandle* handle)
{
    model_description md;
    md.fmiVersion = "2.0";
    md.guid = fmi2_getGuid(handle);
    md.author = fmi2_getAuthor(handle) ? fmi2_getAuthor(handle) : "";
    md.modelName = fmi2_getModelName(handle);
    md.modelIdentifier = fmi2_getModelIdentifier(handle);
    md.description = fmi2_getModelDescription(handle) ? fmi2_getModelDescription(handle) : "";
    md.generationTool = fmi2_getGenerationTool(handle) ? fmi2_getGenerationTool(handle) : "";
    md.generationDateAndTime = fmi2_getGenerationDateAndTime(handle) ? fmi2_getGenerationDateAndTime(handle) : "";

    md.defaultExperiment.startTime = fmi2_getDefaultStartTime(handle);
    md.defaultExperiment.stopTime = fmi2_getDefaultStopTime(handle);
    md.defaultExperiment.stepSize = fmi2_getDefaultStepSize(handle);
    md.defaultExperiment.tolerance = fmi2_getDefaultTolerance(handle);

    const auto varCount = fmi2_getNumberOfVariables(handle);
    for (auto i = 0; i < varCount; i++) {
        const auto var = fmi2_getVariableByIndex(handle, i);
        if (const auto scalar = to_scalar_variable(var)) {
            md.modelVariables.push_back(scalar.value());
        }
    }

    return md;
}

} // namespace fmilibcpp
