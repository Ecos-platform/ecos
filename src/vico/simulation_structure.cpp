
#include "vico/simulation_structure.hpp"

#include "vico/variable_identifier.hpp"

#include <algorithm>
#include <utility>

using namespace vico;

namespace
{

model_description get_model_description(const std::string& instanceName, const std::vector<model_instance_template>& models)
{

    auto result = std::find_if(models.begin(), models.end(), [&instanceName](const model_instance_template& m) {
        return m.instanceName == instanceName;
    });

    if (result == models.end()) {
        throw std::runtime_error("No component named " + instanceName + " found!");
    }

    return result->model->get_model_description();
}

} // namespace

void simulation_structure::add_model(const std::string& name, std::shared_ptr<model> model)
{
    models_.emplace_back(model_instance_template{name, std::move(model)});
}

void simulation_structure::make_connection(const std::string& source, const std::string& target)
{

    variable_identifier vi1(source);
    variable_identifier vi2(target);

    auto md1 = get_model_description(vi1.instanceName, models_);
    auto md2 = get_model_description(vi2.instanceName, models_);

    auto s1 = md1.get_by_name(vi1.variableName);
    if (!s1) {
        throw std::runtime_error("No variable named ...");
    }
    auto s2 = md2.get_by_name(vi2.variableName);
    if (!s2) {
        throw std::runtime_error("No variable named ...");
    }

    if (s1->typeAttribute.index() != s2->typeAttribute.index()) {

        throw std::runtime_error("Variable type mismatch! " + type_name(s1->typeAttribute) + " vs." + type_name(s2->typeAttribute));
    }
}
