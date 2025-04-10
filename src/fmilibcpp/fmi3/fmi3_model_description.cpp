
#include "fmi3_model_description.hpp"

namespace
{

int32_t getStartInt(fmi3VariableHandle* v, fmi3DataType type)
{
    switch (type) {
        case fmi3DataTypeInt8:
            return fmi3_getVariableStartInt16(v);
        case fmi3DataTypeInt16:
            return fmi3_getVariableStartInt16(v);
        case fmi3DataTypeInt32:
            return fmi3_getVariableStartInt32(v);
        case fmi3DataTypeInt64:
            return fmi3_getVariableStartInt64(v);
        case fmi3DataTypeUInt8:
            return fmi3_getVariableStartUInt8(v);
        case fmi3DataTypeUInt16:
            return fmi3_getVariableStartUInt16(v);
        case fmi3DataTypeUInt32:
            return fmi3_getVariableStartUInt32(v);
        case fmi3DataTypeUInt64:
            return fmi3_getVariableStartUInt64(v);
        default:
            throw std::runtime_error("Illegal variable type");
    }
}

double getStartReal(fmi3VariableHandle* v, fmi3DataType type)
{
    switch (type) {
        case fmi3DataTypeFloat32:
            return fmi3_getVariableStartFloat32(v);
        case fmi3DataTypeFloat64:
            return fmi3_getVariableStartFloat64(v);
        default:
            throw std::runtime_error("Illegal variable type");
    }
}

std::optional<std::string> fmi3CausalityToString(fmi3Causality causality)
{
    switch (causality) {
        case fmi3CausalityInput: return "Input";
        case fmi3CausalityOutput: return "Output";
        case fmi3CausalityParameter: return "Parameter";
        case fmi3CausalityCalculatedParameter: return "CalculatedParameter";
        case fmi3CausalityLocal: return "Local";
        case fmi3CausalityIndependent: return "Independent";
        default: return std::nullopt;
    }
}

std::optional<std::string> fmi3VariabilityToString(fmi3Variability variability)
{
    switch (variability) {
        case fmi3VariabilityFixed: return "Fixed";
        case fmi3VariabilityTunable: return "Tunable";
        case fmi3VariabilityConstant: return "Constant";
        case fmi3VariabilityDiscrete: return "Discrete";
        case fmi3VariabilityContinuous: return "Continuous";
        default: return std::nullopt;
    }
}

std::optional<fmilibcpp::scalar_variable> to_scalar_variable(fmi3VariableHandle* v)
{
    const auto type = fmi3_getVariableDataType(v);
    if (type == fmi3DataTypeEnumeration) {
        return std::nullopt;
    }

    fmilibcpp::scalar_variable var;
    var.vr = fmi3_getVariableValueReference(v);
    var.name = fmi3_getVariableName(v);
    var.description = fmi3_getVariableDescription(v) ? fmi3_getVariableDescription(v) : "";
    var.causality = fmi3CausalityToString(fmi3_getVariableCausality(v));
    var.variability = fmi3VariabilityToString(fmi3_getVariableVariability(v));

    switch (type) {
        case fmi3DataTypeFloat32:
        case fmi3DataTypeFloat64: {
            fmilibcpp::real_attributes r{};
            if (fmi3_getVariableHasStartValue(v)) {
                r.start = getStartReal(v, type);
            }
            var.typeAttributes = r;
        } break;
        case fmi3DataTypeInt8:
        case fmi3DataTypeInt16:
        case fmi3DataTypeInt32:
        case fmi3DataTypeInt64:
        case fmi3DataTypeUInt8:
        case fmi3DataTypeUInt16:
        case fmi3DataTypeUInt32:
        case fmi3DataTypeUInt64: {
            fmilibcpp::integer_attributes i{};
            if (fmi3_getVariableHasStartValue(v)) {
                i.start = getStartInt(v, type);
            }
            var.typeAttributes = i;
        } break;
        case fmi3DataTypeBoolean: {
            fmilibcpp::boolean_attributes b{};
            if (fmi3_getVariableHasStartValue(v)) {
                b.start = fmi3_getVariableStartBoolean(v);
            }
            var.typeAttributes = b;
        } break;
        case fmi3DataTypeString: {
            fmilibcpp::string_attributes s{};
            if (fmi3_getVariableHasStartValue(v)) {
                s.start = fmi3_getVariableStartString(v);
            }
            var.typeAttributes = s;
        } break;
        default: break;
    }
    return var;
}

} // namespace

namespace fmilibcpp
{

model_description create_fmi3_model_description(fmiHandle* handle)
{
    model_description md;
    md.fmiVersion = fmi3_getVersion(handle) ? fmi3_getVersion(handle) : "3.0";
    // md.guid = fmi3_getGuid(handle);
    md.author = fmi3_author(handle) ? fmi3_author(handle) : "";
    md.modelName = fmi3_modelName(handle);
    md.modelIdentifier = fmi3cs_getModelIdentifier(handle);
    md.description = fmi3_description(handle) ? fmi3_description(handle) : "";
    md.generationTool = fmi3_generationTool(handle) ? fmi3_generationTool(handle) : "";
    md.generationDateAndTime = fmi3_generationDateAndTime(handle) ? fmi3_generationDateAndTime(handle) : "";

    md.canGetAndSetState = fmi3cs_getCanGetAndSetFMUState(handle);

    md.defaultExperiment.startTime = fmi3_getDefaultStartTime(handle);
    md.defaultExperiment.stopTime = fmi3_getDefaultStopTime(handle);
    md.defaultExperiment.stepSize = fmi3_getDefaultStepSize(handle);
    md.defaultExperiment.tolerance = fmi3_getDefaultTolerance(handle);

    const auto varCount = fmi3_getNumberOfVariables(handle);
    for (auto i = 0; i < varCount; i++) {
        const auto var = fmi3_getVariableByIndex(handle, i+1);
        if (const auto scalar = to_scalar_variable(var)) {
            md.modelVariables.push_back(scalar.value());
        }
    }

    return md;
}

} // namespace fmilibcpp
