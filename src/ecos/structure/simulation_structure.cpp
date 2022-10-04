
#include "ecos/structure/simulation_structure.hpp"

#include "ecos/variable_identifier.hpp"

#include <utility>

using namespace ecos;

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

simulation_structure::simulation_structure()
    : resolver_(default_model_resolver())
{
}

void simulation_structure::add_model(const std::string& instanceName, const std::string& uri)
{
    auto model = resolver_->resolve(uri);
    add_model(instanceName, model);
}

void simulation_structure::add_model(const std::string& instanceName, const std::filesystem::path& path)
{
    add_model(instanceName, std::filesystem::relative(path).string());
}

void simulation_structure::add_model(const std::string& instanceName, std::shared_ptr<model> model)
{
    if (models_.count(instanceName)) {
        throw std::runtime_error("A model named " + instanceName + " has already been added!");
    }
    models_[instanceName] = std::move(model);
}

std::unique_ptr<simulation> simulation_structure::load(std::unique_ptr<algorithm> algorithm)
{
    std::unordered_map<std::string, std::unique_ptr<model_instance>> instances;
    for (auto& [name, model] : models_) {
        instances.emplace(name, model->instantiate(name));
    }

    for (auto& [parameterSetName, map] : parameterSets) {
        for (auto& [v, value] : map) {
            instances[v.instanceName]->add_parameterset_entry(parameterSetName, v.variableName, value);
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
