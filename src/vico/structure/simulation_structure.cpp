
#include "vico/structure/simulation_structure.hpp"

#include "vico/structure/variable_identifier.hpp"

#include <fmilibcpp/model_description.hpp>
#include <utility>

using namespace vico;

namespace
{

template<class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// fmilibcpp::model_description get_model_description(const std::string& instanceName, const std::vector<model>& models)
//{
//     auto result = std::find_if(models.begin(), models.end(), [&instanceName](const model_instance_template& m) {
//         return m.instanceName == instanceName;
//     });
//
//     if (result == models.end()) {
//         throw std::runtime_error("No component named '" + instanceName + "' found!");
//     }
//
//     return result->get_model_description();
// }

} // namespace

void simulation_structure::add_model(const std::string& instanceName, std::shared_ptr<model> model)
{
    models_.emplace_back(instanceName, std::move(model));
}

// void simulation_structure::make_connection(const variable_identifier& source, const variable_identifier& sink)
//{
//
//     auto md1 = get_model_description(vi1.instanceName, models_);
//     auto md2 = get_model_description(vi2.instanceName, models_);
//
//     auto s1 = md1.get_by_name(vi1.variableName);
//     if (!s1) {
//         throw std::runtime_error("No variable named '" + vi1.variableName + "'");
//     }
//     auto s2 = md2.get_by_name(vi2.variableName);
//     if (!s2) {
//         throw std::runtime_error("No variable named '" + vi2.variableName + "'");
//     }
//
//     if (s1->typeAttributes.index() != s2->typeAttributes.index()) {
//         throw std::runtime_error("Variable type mismatch! " + type_name(s1->typeAttributes) + " vs." + type_name(s2->typeAttributes));
//     }
//
//        std::visit([&](auto&& arg) {
//            using T = std::decay_t<decltype(arg)>;
//            if constexpr (std::is_same_v<T, fmilibcpp::integer_attributes>) {
//                unbound_connection_t<int> c(source, sink);
//                connections_.emplace_back(c);
//            } else if constexpr (std::is_same_v<T, fmilibcpp::real_attributes>) {
//                unbound_connection_t<double> c(source, sink);
//                connections_.emplace_back(c);
//            } else if constexpr (std::is_same_v<T, fmilibcpp::string_attributes>) {
//                unbound_connection_t<std::string> c(source, sink);
//                connections_.emplace_back(c);
//            } else if constexpr (std::is_same_v<T, fmilibcpp::boolean_attributes>) {
//                unbound_connection_t<bool> c(source, sink);
//                connections_.emplace_back(c);
//            }
//        },
//            s1->typeAttributes);
//}

std::unique_ptr<simulation> simulation_structure::load(std::unique_ptr<algorithm> algorithm)
{
    auto sim = std::make_unique<simulation>(std::move(algorithm));
    for (auto& [name, model] : models_) {
        sim->add_slave(model->instantiate(name));
    }
    for (auto& connection : connections_) {
        std::visit(overloaded{
                       [&sim](int_connection& arg) {
                           auto c = sim->add_connection<int>(arg.source, arg.sink);
                           if (arg.modifier) c->modifier = [arg](int value) {
                               return (*arg.modifier)(value);
                           };
                       },
                       [&sim](real_connection& arg) {
                           sim->add_connection<double>(arg.source, arg.sink);
                       },
                       [&sim](string_connection& arg) {
                           sim->add_connection<double>(arg.source, arg.sink);
                       },
                       [&sim](bool_connection& arg) {
                           sim->add_connection<double>(arg.source, arg.sink);
                       }},
            connection);
    }
    return sim;
}
