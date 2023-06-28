
#include "ecos/ssp/ssp_loader.hpp"

#include "ssp.hpp"

using namespace ecos;

namespace
{

class ssp_based_simulation_structure : public simulation_structure
{
public:
    explicit ssp_based_simulation_structure(const std::filesystem::path& path)
        : desc_(path)
    {

        const auto& system = desc_.system;
        const auto& parameterSets = system.elements.parameterSets;
        const auto& components = system.elements.components;
        const auto& connections = system.connections;

        auto resolver = default_model_resolver();
        for (const auto& [name, component] : components) {
            auto model = resolver->resolve(desc_.dir(), component.source);
            add_model(name, model);
        }

        for (const auto& connection : connections) {
            const auto& startComponent = components.at(connection.startElement);
            const auto& startConnector = startComponent.connectors.at(connection.startConnector);
            const auto& endComponent = components.at(connection.endElement);
            const auto& endConnector = endComponent.connectors.at(connection.endConnector);
            const variable_identifier source(connection.startElement, connection.startConnector);
            const variable_identifier sink(connection.endElement, connection.endConnector);

            if (!(startConnector.type == endConnector.type)) {
                throw std::runtime_error("Incompatible connector types!");
            }

            const auto typeName = startConnector.type.typeName();
            if (typeName == "Real") {
                std::optional<std::function<double(double)>> f;
                if (connection.linearTransformation) {
                    f = [connection](double value) {
                        return value * connection.linearTransformation->factor + connection.linearTransformation->offset;
                    };
                }
                make_connection<double>(source, sink, f);
            } else if (typeName == "Integer") {
                make_connection<int>(source, sink);
            } else if (typeName == "Boolean") {
                make_connection<bool>(source, sink);
            } else if (typeName == "String") {
                make_connection<std::string>(source, sink);
            }
        }

        for (const auto& [parameterSetName, sets] : parameterSets) {
            std::map<variable_identifier, scalar_value> map;
            for (const auto& [component, parameters] : sets) {
                for (const auto& p : parameters) {
                    variable_identifier v{component.name, p.name};
                    map[v] = p.type.value;
                }
            }
            if (!map.empty()) {
                add_parameter_set(parameterSetName, map);
            }
        }
    }

private:
    ssp::SystemStructureDescription desc_;
};

} // namespace

std::unique_ptr<simulation_structure> ecos::load_ssp(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("No such file: '" + std::filesystem::absolute(path).string() + "'");
    }

    return std::make_unique<ssp_based_simulation_structure>(path);
}