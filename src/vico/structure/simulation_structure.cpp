
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

} // namespace

void simulation_structure::add_model(const std::string& instanceName, std::shared_ptr<model> model)
{
    models_.emplace_back(instanceName, std::move(model));
}

std::unique_ptr<simulation> simulation_structure::load(std::unique_ptr<algorithm> algorithm, std::optional<std::string> parameterSet)
{
    auto sim = std::make_unique<simulation>(std::move(algorithm));
    for (auto& [name, model] : models_) {
        sim->add_slave(model->instantiate(name));
    }
    if (parameterSet) {
        auto& set = parameterSets.at(*parameterSet);
        for (auto& [v, value] : set) {
            variable_identifier v1 = v;
            std::visit(overloaded{
                           [&](double arg) { sim->get_property<double>(v1)->set_value(arg); },
                           [&](int arg) { sim->get_property<int>(v1)->set_value(arg); },
                           [&](bool arg) { sim->get_property<bool>(v1)->set_value(arg); },
                           [&](const std::string& arg) { sim->get_property<std::string>(v1)->set_value(arg); }},
                value);
        }
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
