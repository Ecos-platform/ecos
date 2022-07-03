
#include "vico/ssp/ssp_loader.hpp"

#include <fmilibcpp/fmu.hpp>
#include <ssp/ssp.hpp>

using namespace vico;


simulation_structure vico::load_ssp(const fs::path& path)
{
    simulation_structure ss;
    ssp::SystemStructureDescription desc(path);

    const auto& system = desc.system;
    const auto& components = system.elements.components;
    const auto& connections = system.connections;

    for (const auto& [name, component] : components) {
        ss.add_model(name, fmilibcpp::loadFmu(desc.file(component.source)));
    }

    for (const auto& connection : connections) {
        const std::string source(connection.startElement + "." + connection.startConnector);
        const std::string sink(connection.endElement + "." + connection.endConnector);
        ss.make_connection(source, sink);
    }

    return ss;
}