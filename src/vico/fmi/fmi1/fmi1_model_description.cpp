
#include "fmi1_model_description.hpp"

using namespace vico;

namespace
{

std::optional<scalar_variable> to_scalar_variable(fmi1_import_variable_t* v)
{
    const auto type = fmi1_import_get_variable_base_type(v);
    if (type == fmi1_base_type_enum) {
        return std::nullopt;
    }

    scalar_variable var;
    var.valueRef = fmi1_import_get_variable_vr(v);
    var.name = fmi1_import_get_variable_name(v);
    //var.description = fmi1_import_get_variable_description(v);
    var.causality = fmi1_causality_to_string(fmi1_import_get_causality(v));
    var.variability = fmi1_variability_to_string(fmi1_import_get_variability(v));

    fmi1_import_get_causality(v);

    switch (type) {
        case fmi1_base_type_real: {
            real r;
            if (fmi1_import_get_variable_has_start(v)) {
                r.start = fmi1_import_get_real_variable_start(fmi1_import_get_variable_as_real(v));
            }
            var.typeAttribute = r;
        } break;
        case fmi1_base_type_int: {
            integer i;
            if (fmi1_import_get_variable_has_start(v)) {
                i.start = fmi1_import_get_integer_variable_start(fmi1_import_get_variable_as_integer(v));
            }
            var.typeAttribute = i;
        } break;
        case fmi1_base_type_bool: {
            boolean b;
            if (fmi1_import_get_variable_has_start(v)) {
                b.start = fmi1_import_get_boolean_variable_start(fmi1_import_get_variable_as_boolean(v));
            }
            var.typeAttribute = b;
        } break;
        case fmi1_base_type_str: {
            string s;
            if (fmi1_import_get_variable_has_start(v)) {
                s.start = fmi1_import_get_string_variable_start(fmi1_import_get_variable_as_string(v));
            }
            var.typeAttribute = s;
        } break;
        case fmi1_base_type_enum: break;
    }
    return var;
}

} // namespace

model_description vico::create_model_description(fmi1_import_t* handle)
{
    model_description md;
//    md.fmiVersion = "1.0";
    md.guid = fmi1_import_get_GUID(handle);
//    md.author = fmi1_import_get_author(handle);
    md.modelName = fmi1_import_get_model_name(handle);
    md.modelIdentifier = fmi1_import_get_model_identifier(handle);
    md.description = fmi1_import_get_description(handle);
//    md.generationTool = fmi1_import_get_generation_tool(handle);
//    md.generationDateAndTime = fmi1_import_get_generation_date_and_time(handle);

    md.defaultExperiment.startTime = fmi1_import_get_default_experiment_start(handle);
    md.defaultExperiment.stopTime = fmi1_import_get_default_experiment_stop(handle);
    md.defaultExperiment.tolerance = fmi1_import_get_default_experiment_tolerance(handle);

    const auto varList = fmi1_import_get_variable_list(handle);
    const auto varCount = fmi1_import_get_variable_list_size(varList);
    for (auto i = 0; i < varCount; i++) {
        const auto var = fmi1_import_get_variable(varList, i);
        const auto scalar = to_scalar_variable(var);
        if (scalar) {
            md.modelVariables.push_back(scalar.value());
        }
    }

    fmi1_import_free_variable_list(varList);

    return md;
}
