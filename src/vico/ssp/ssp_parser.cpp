
#include "vico/ssp/ssp_parser.hpp"

#include <vico/util/temp_dir.hpp>
#include <vico/util/unzipper.hpp>

#include <string>
#include <optional>

#include <pugixml.hpp>

using namespace vico;

namespace {

// clang-format off
struct Real{};
struct Integer{};
struct Boolean{};
struct String{};
// clang-format on

struct Type {
    std::optional<Real> real;
    std::optional<Integer> integer;
    std::optional<Boolean> boolean;
    std::optional<String> string;

    bool isReal() {return real.has_value();}
    bool isInteger() {return integer.has_value();}
    bool isBool() {return boolean.has_value();}
    bool isString() {return string.has_value();}
};

struct Connector {
    std::string name;
    std::string kind;
    Type type;
};

struct Component {
    std::string name;
    std::string source;
    std::vector<Connector> connectors;
};

struct Elements {
    std::vector<Component> components;
};

struct LinearTransformation {
    double factor{0};
    double offset{0};
};

struct Connection {
    std::string startElement;
    std::string startConnector;

    std::string endElement;
    std::string endConnector;

    std::optional<LinearTransformation> linearTransformation;
};

struct System {
    std::string name;
    std::string description;

    Elements elements;
    std::vector<Connection> connections;
};

struct SystemStructureDescription {
    std::string name;
    std::string version;
    System system;
};


}

simulation_structure vico::parse_ssp(const fs::path& path)
{

    if (!exists(path)) {
        throw std::runtime_error("No such file: " + absolute(path).string());
    }

    temp_dir dir("ssp");
    unzip(path, dir.path());

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(fs::path(dir.path() / "SystemStructure.ssd").c_str());
    if (!result) {
        throw std::runtime_error("Unable to parse SystemStructure.ssd");
    }

    SystemStructureDescription desc;

    const auto root = doc.child("ssd:SystemStructureDescription");
    desc.name = root.attribute("name").as_string();
    desc.version = root.attribute("version").as_string();

    if (desc.version != "1.0") {
        throw std::runtime_error("Unsupported SSP version: " + desc.version);
    }

    const auto system_node = root.child("ssd:System");
    desc.system.name = system_node.attribute("name").as_string();
    desc.system.description = system_node.attribute("description").as_string();

    const auto elements_node = system_node.child("ssd:Elements");

    for (const auto child_node : elements_node) {
        if (std::string(child_node.name()) == "ssd:Component") {
            const auto componentName = child_node.attribute("name").as_string();
            const auto componentSource = child_node.attribute("source").as_string();
            Component comp{componentName, componentSource};
            for (auto connector_node : child_node) {
                const auto connectorName = connector_node.attribute("name").as_string();
                const auto connectorKind = connector_node.attribute("kind").as_string();
                Connector connector{connectorName, connectorKind};
                if (connector_node.child("ssv:Real")) {
                    connector.type.real = Real();
                } else if (connector_node.child("ssv:Integer")) {
                    connector.type.integer = Integer();
                } else if (connector_node.child("ssv:Boolean")) {
                    connector.type.boolean = Boolean();
                } else if (connector_node.child("ssv:String")) {
                    connector.type.string = String();
                }
                comp.connectors.emplace_back(connector);
            }
            desc.system.elements.components.emplace_back(comp);
        }
    }

    const auto connections_node = system_node.child("connections");
    for (const auto connection_node : connections_node) {
        const auto startElement = connection_node.attribute("startElement").as_string();
        const auto startConnector = connection_node.attribute("startConnector").as_string();
        const auto endElement = connection_node.attribute("endElement").as_string();
        const auto endConnector = connection_node.attribute("endConnector").as_string();
        Connection c{startElement, startConnector, endElement, endConnector};
        desc.system.connections.emplace_back(c);
    }


    return {};
}