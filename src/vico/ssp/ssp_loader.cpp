
#include "vico/ssp/ssp_loader.hpp"

#include <vico/fmi/fmi_model.hpp>

#include <ssp/ssp.hpp>

using namespace vico;


simulation_structure vico::load_ssp(const fs::path& path)
{

    if (!fs::exists(path)) {
        throw std::runtime_error("No such file: '" + fs::absolute(path).string() + "'");
    }

    simulation_structure ss;
    ssp::SystemStructureDescription desc(path);

    const auto& system = desc.system;
    const auto& components = system.elements.components;
    const auto& connections = system.connections;

    std::unordered_map<std::string, std::shared_ptr<model>> modelCache;

    for (const auto& [name, component] : components) {
        const auto fmuFile = desc.file(component.source);
        std::shared_ptr<model> model;
        if (modelCache.count(fmuFile.string())) {
            model = modelCache[fmuFile.string()];
        } else {
            model = std::make_shared<fmi_model>(fmuFile);
            modelCache[fmuFile.string()] = model;
        }
        ss.add_model(name, model);
    }

    for (const auto& connection : connections) {
        const auto startComponent = components.at(connection.startElement);
        const auto startConnector = startComponent.connectors.at(connection.startConnector);
        const auto endComponent = components.at(connection.endElement);
        const auto endConnector = endComponent.connectors.at(connection.endConnector);
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
                    return value + connection.linearTransformation->offset * connection.linearTransformation->factor;
                };
            }
            ss.make_connection<double>(source, sink, f);
        } else if (typeName == "Integer") {
            ss.make_connection<int>(source, sink);
        } else if (typeName == "Boolean") {
            ss.make_connection<bool>(source, sink);
        } else if (typeName == "String") {
            ss.make_connection<std::string>(source, sink);
        }
    }

    return ss;
}