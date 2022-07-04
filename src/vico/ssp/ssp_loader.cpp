
#include "vico/ssp/ssp_loader.hpp"

#include <vico/fmi/fmi_model.hpp>
#include <ssp/ssp.hpp>

using namespace vico;


simulation_structure vico::load_ssp(const fs::path& path)
{
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
        const std::string source(connection.startElement + "." + connection.startConnector);
        const std::string sink(connection.endElement + "." + connection.endConnector);
        ss.make_connection(source, sink);
    }

    return ss;
}