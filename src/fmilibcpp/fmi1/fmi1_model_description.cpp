
#include "fmi1_model_description.hpp"

#include <fmi4c_types_fmi1.h>

namespace
{

std::optional<std::string> fmi1CausalityToString(fmi1Causality causality)
{
    switch (causality) {
        case fmi1CausalityInput:
            return "Input";
        case fmi1CausalityOutput:
            return "Output";
        case fmi1CausalityInternal:
            return "Internal";
        case fmi1CausalityNone:
            return "None";
        default:
            return std::nullopt;
    }
}

std::optional<std::string> fmi1VariabilityToString(fmi1Variability variability)
{
    switch (variability) {
        case fmi1VariabilityConstant:
            return "Constant";
        case fmi1VariabilityParameter:
            return "Parameter";
        case fmi1VariabilityDiscrete:
            return "Discrete";
        case fmi1VariabilityContinuous:
            return "Continuous";
        default:
            return std::nullopt;
    }
}

std::optional<fmilibcpp::scalar_variable> to_scalar_variable(fmi1VariableHandle* v)
{
    const auto type = fmi1_getVariableDataType(v);
    if (type == fmi1DataTypeEnumeration) {
        return std::nullopt;
    }

    fmilibcpp::scalar_variable var;
    var.vr = fmi1_getVariableValueReference(v);
    var.name = fmi1_getVariableName(v);
    // var.description = fmi1_import_get_variable_description(v);
    var.causality = fmi1CausalityToString(fmi1_getVariableCausality(v));
    var.variability = fmi1VariabilityToString(fmi1_getVariableVariability(v));

    switch (type) {
        case fmi1DataTypeReal: {
            fmilibcpp::real_attributes r{};
            if (false) {
                r.start = fmi1_getVariableStartReal(v);
            }
            var.typeAttributes = r;
        } break;
        case fmi1DataTypeInteger: {
            fmilibcpp::integer_attributes i{};
            if (false) {
                i.start = fmi1_getVariableStartInteger(v);
            }
            var.typeAttributes = i;
        } break;
        case fmi1DataTypeBoolean: {
            fmilibcpp::boolean_attributes b{};
            if (false) {
                b.start = fmi1_getVariableStartBoolean(v);
            }
            var.typeAttributes = b;
        } break;
        case fmi1DataTypeString: {
            fmilibcpp::string_attributes s{};
            if (false) {
                s.start = fmi1_getVariableStartString(v);
            }
            var.typeAttributes = s;
        } break;
        case fmi1DataTypeEnumeration: break;
    }
    return var;
}

} // namespace

namespace fmilibcpp
{

model_description create_model_description(fmiHandle* handle)
{
    model_description md;
    md.fmiVersion = "1.0";
    // md.guid = handle->fmi1.guid;
    // md.author = fmi1_import_get_author(handle);
    // md.modelName = fmi1_import_get_model_name(handle);
    // md.modelIdentifier = fmi1_import_get_model_identifier(handle);
    // md.description = fmi1_import_get_description(handle);
    // md.generationTool = fmi1_import_get_generation_tool(handle);
    // md.generationDateAndTime = fmi1_import_get_generation_date_and_time(handle);
    //
    md.defaultExperiment.startTime = fmi1_getDefaultStartTime(handle);
    md.defaultExperiment.stopTime = fmi1_getDefaultStopTime(handle);
    md.defaultExperiment.tolerance = fmi1_getDefaultTolerance(handle);


    const auto varCount = fmi1_getNumberOfVariables(handle);
    for (auto i = 0; i < varCount; i++) {
        const auto var = fmi1_getVariableByIndex(handle, i);
        if (const auto scalar = to_scalar_variable(var)) {
            md.modelVariables.push_back(scalar.value());
        }
    }

    return md;
}

} // namespace fmilibcpp
