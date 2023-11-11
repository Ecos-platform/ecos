
#include "fmi2_model_description.hpp"

namespace
{

std::optional<fmilibcpp::scalar_variable> to_scalar_variable(fmi2_import_variable_t* v)
{
    const auto type = fmi2_import_get_variable_base_type(v);
    if (type == fmi2_base_type_enum) {
        return std::nullopt;
    }

    fmilibcpp::scalar_variable var;
    var.vr = fmi2_import_get_variable_vr(v);
    var.name = fmi2_import_get_variable_name(v);
    //    var.description = fmi2_import_get_variable_description(v);
    var.causality = fmi2_causality_to_string(fmi2_import_get_causality(v));
    var.variability = fmi2_variability_to_string(fmi2_import_get_variability(v));

    switch (type) {
        case fmi2_base_type_real: {
            fmilibcpp::real_attributes r{};
            if (fmi2_import_get_variable_has_start(v)) {
                r.start = fmi2_import_get_real_variable_start(fmi2_import_get_variable_as_real(v));
            }
            var.typeAttributes = r;
        } break;
        case fmi2_base_type_int: {
            fmilibcpp::integer_attributes i{};
            if (fmi2_import_get_variable_has_start(v)) {
                i.start = fmi2_import_get_integer_variable_start(fmi2_import_get_variable_as_integer(v));
            }
            var.typeAttributes = i;
        } break;
        case fmi2_base_type_bool: {
            fmilibcpp::boolean_attributes b{};
            if (fmi2_import_get_variable_has_start(v)) {
                b.start = fmi2_import_get_boolean_variable_start(fmi2_import_get_variable_as_boolean(v));
            }
            var.typeAttributes = b;
        } break;
        case fmi2_base_type_str: {
            fmilibcpp::string_attributes s{};
            if (fmi2_import_get_variable_has_start(v)) {
                s.start = fmi2_import_get_string_variable_start(fmi2_import_get_variable_as_string(v));
            }
            var.typeAttributes = s;
        } break;
        case fmi2_base_type_enum: break;
    }
    return var;
}

} // namespace

namespace fmilibcpp
{

model_description create_model_description(fmi2_import_t* handle)
{
    model_description md;
    md.fmiVersion = "2.0";
    md.guid = fmi2_import_get_GUID(handle);
    md.author = fmi2_import_get_author(handle);
    md.modelName = fmi2_import_get_model_name(handle);
    md.modelIdentifier = fmi2_import_get_model_identifier_CS(handle);
    md.description = fmi2_import_get_description(handle);
    md.generationTool = fmi2_import_get_generation_tool(handle);
    md.generationDateAndTime = fmi2_import_get_generation_date_and_time(handle);

    md.defaultExperiment.startTime = fmi2_import_get_default_experiment_start(handle);
    md.defaultExperiment.stopTime = fmi2_import_get_default_experiment_stop(handle);
    md.defaultExperiment.stepSize = fmi2_import_get_default_experiment_step(handle);
    md.defaultExperiment.tolerance = fmi2_import_get_default_experiment_tolerance(handle);

    const auto varList = fmi2_import_get_variable_list(handle, 0);
    const auto varCount = fmi2_import_get_variable_list_size(varList);
    for (auto i = 0; i < varCount; i++) {
        const auto var = fmi2_import_get_variable(varList, i);
        const auto scalar = to_scalar_variable(var);
        if (scalar) {
            md.modelVariables.push_back(scalar.value());
        }
    }

    fmi2_import_free_variable_list(varList);

    return md;
}

} // namespace fmilibcpp