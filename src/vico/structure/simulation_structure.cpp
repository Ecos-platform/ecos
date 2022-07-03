
#include "vico/structure/simulation_structure.hpp"

#include "vico/structure/variable_identifier.hpp"

#include <algorithm>
#include <fmilibcpp/fmu.hpp>
#include <fmilibcpp/model_description.hpp>
#include <utility>

using namespace vico;

namespace
{

fmilibcpp::model_description get_model_description(const std::string& instanceName, const std::vector<model_instance_template>& models)
{
    auto result = std::find_if(models.begin(), models.end(), [&instanceName](const model_instance_template& m) {
        return m.instanceName == instanceName;
    });

    if (result == models.end()) {
        throw std::runtime_error("No component named '" + instanceName + "' found!");
    }

    return result->get_model_description();
}

} // namespace

void simulation_structure::add_model(const std::string& instanceName, std::shared_ptr<fmilibcpp::fmu> model)
{
    models_.emplace_back(model_instance_template{instanceName, std::move(model)});
}

void simulation_structure::make_connection(const variable_identifier& vi1, const variable_identifier& vi2)
{

    auto md1 = get_model_description(vi1.instanceName, models_);
    auto md2 = get_model_description(vi2.instanceName, models_);

    auto s1 = md1.get_by_name(vi1.variableName);
    if (!s1) {
        throw std::runtime_error("No variable named '" + vi1.variableName + "'");
    }
    auto s2 = md2.get_by_name(vi2.variableName);
    if (!s2) {
        throw std::runtime_error("No variable named '" + vi2.variableName + "'");
    }

    if (s1->typeAttributes.index() != s2->typeAttributes.index()) {
        throw std::runtime_error("Variable type mismatch! " + type_name(s1->typeAttributes) + " vs." + type_name(s2->typeAttributes));
    }

    //    std::visit([&](auto&& arg) {
    //        using T = std::decay_t<decltype(arg)>;
    //        if constexpr (std::is_same_v<T, integer>) {
    //            unbound_connector<int> source{vi1.instanceName, vi1.variableName};
    //            unbound_connector<int> sink{vi2.instanceName, vi2.variableName};
    //            unbound_connection_t<int> c(source, sink);
    //            connections_.emplace_back(c);
    //        } else if constexpr (std::is_same_v<T, real>) {
    //            unbound_connector<double> source{vi1.instanceName, vi1.variableName};
    //            unbound_connector<double> sink{vi2.instanceName, vi2.variableName};
    //            unbound_connection_t<double> c(source, sink);
    //            connections_.emplace_back(c);
    //        } else if constexpr (std::is_same_v<T, string>) {
    //            unbound_connector<std::string> source{vi1.instanceName, vi1.variableName};
    //            unbound_connector<std::string> sink{vi2.instanceName, vi2.variableName};
    //            unbound_connection_t<std::string> c(source, sink);
    //            connections_.emplace_back(c);
    //        } else if constexpr (std::is_same_v<T, boolean>) {
    //            unbound_connector<bool> source{vi1.instanceName, vi1.variableName};
    //            unbound_connector<bool> sink{vi2.instanceName, vi2.variableName};
    //            unbound_connection_t<bool> c(source, sink);
    //            connections_.emplace_back(c);
    //        }
    //    },
    //        s1->typeAttribute);

    // connections_.emplace_back({vi1.instanceName, vi1.variableName});
}
