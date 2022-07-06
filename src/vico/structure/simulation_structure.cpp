
#include "vico/structure/simulation_structure.hpp"

#include "vico/variable_identifier.hpp"

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

std::unique_ptr<simulation> simulation_structure::load(std::unique_ptr<algorithm> algorithm)
{

    std::unordered_map<std::string, std::unique_ptr<model_instance>> instances;
    for (auto& [name, model] : models_) {
        instances.emplace(name, model->instantiate(name));
    }

    for (auto& [parameterSetName, map] : parameterSets) {
        for (auto& [v, value] : map) {
            auto& props = instances[v.instanceName]->get_properties();
            switch (value.index()) {
                case 0: {
                    props.get_real_property(v.variableName)->set_value(std::get<double>(value));
                    break;
                }
                case 1: {
                    props.get_int_property(v.variableName)->set_value(std::get<int>(value));
                    break;
                }
                case 2: {
                    props.get_bool_property(v.variableName)->set_value(std::get<bool>(value));
                    break;
                }
                case 3: {
                    props.get_string_property(v.variableName)->set_value(std::get<std::string>(value));
                    break;
                }
            }
        }
    }

    auto sim = std::make_unique<simulation>(std::move(algorithm));
    for (auto& [name, instance] : instances) {
        sim->add_slave(std::move(instance));
    }

    for (auto& connection : connections_) {
        std::visit(overloaded{
                       [&sim](unbound_int_connection& arg) {
                           sim->make_int_connection(arg.source, arg.sink);
                       },
                       [&sim](unbound_real_connection& arg) {
                           auto c = sim->make_real_connection(arg.source, arg.sink);
                           if (arg.modifier) c->modifier = [arg](double value) {
                               return (*arg.modifier)(value);
                           };
                       },
                       [&sim](unbound_bool_connection& arg) {
                           sim->make_bool_connection(arg.source, arg.sink);
                       },
                       [&sim](unbound_string_connection& arg) {
                           sim->make_string_connection(arg.source, arg.sink);
                       }},
            connection);
    }
    return sim;
}
