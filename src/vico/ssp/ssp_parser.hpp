
#ifndef VICO_SSP_PARSER_HPP
#define VICO_SSP_PARSER_HPP

#include "vico/ssp/ssp_loader.hpp"
#include <vico/util/temp_dir.hpp>
#include <vico/util/unzipper.hpp>

#include <optional>
#include <pugixml.hpp>
#include <string>

using namespace vico;

namespace
{

// clang-format off
struct Real{};
struct Integer{};
struct Boolean{};
struct String{};
// clang-format on

struct Type
{
    std::optional<Real> real;
    std::optional<Integer> integer;
    std::optional<Boolean> boolean;
    std::optional<String> string;

    bool isReal() { return real.has_value(); }
    bool isInteger() { return integer.has_value(); }
    bool isBool() { return boolean.has_value(); }
    bool isString() { return string.has_value(); }
};

struct Connector
{
    std::string name;
    std::string kind;
    Type type;
};

struct Component
{
    std::string name;
    std::string source;
    std::vector<Connector> connectors;
};

struct Elements
{
    std::vector<Component> components;
};

struct LinearTransformation
{
    double factor{0};
    double offset{0};
};

struct Connection
{
    std::string startElement;
    std::string startConnector;

    std::string endElement;
    std::string endConnector;

    std::optional<LinearTransformation> linearTransformation;
};

struct System
{
    std::string name;
    std::string description;

    Elements elements;
    std::vector<Connection> connections;
};

struct SystemStructureDescription
{
    std::string name;
    std::string version;
    System system;
};


} // namespace

Connection parse_connection(const pugi::xml_node& node)
{
    const auto startElement = node.attribute("startElement").as_string();
    const auto startConnector = node.attribute("startConnector").as_string();
    const auto endElement = node.attribute("endElement").as_string();
    const auto endConnector = node.attribute("endConnector").as_string();
    Connection c = {startElement, startConnector, endElement, endConnector};
    const auto transformationNode = node.child("ssc:LinearTransformation");
    if (transformationNode) {
        double factor = transformationNode.attribute("factor").as_double();
        double offset = transformationNode.attribute("offset").as_double();
        c.linearTransformation = {factor, offset};
    }
    return c;
}

std::vector<Connection> parse_connections(const pugi::xml_node& node)
{
    std::vector<Connection> connections;
    for (const auto c : node) {
        connections.emplace_back(parse_connection(c));
    }
    return connections;
}

Connector parse_connector(const pugi::xml_node& node)
{
    const auto connectorName = node.attribute("name").as_string();
    const auto connectorKind = node.attribute("kind").as_string();
    Connector connector = {connectorName, connectorKind};
    if (node.child("ssc:Real")) {
        connector.type.real = Real();
    } else if (node.child("ssc:Integer")) {
        connector.type.integer = Integer();
    } else if (node.child("ssc:Boolean")) {
        connector.type.boolean = Boolean();
    } else if (node.child("ssc:String")) {
        connector.type.string = String();
    }
    return connector;
}

std::vector<Connector> parse_connectors(const pugi::xml_node& node)
{
    std::vector<Connector> connectors;
    for (const auto c : node) {
        connectors.emplace_back(parse_connector(c));
    }
    return connectors;
}

Component parse_component(const pugi::xml_node& node)
{
    const auto componentName = node.attribute("name").as_string();
    const auto componentSource = node.attribute("source").as_string();
    const auto connectors = parse_connectors(node.child("ssd:Connectors"));
    return {componentName, componentSource, connectors};
}

std::vector<Component> parse_components(const pugi::xml_node& node)
{
    std::vector<Component> components;
    for (const auto child_node : node) {
        if (std::string(child_node.name()) == "ssd:Component") {
            components.emplace_back(parse_component(child_node));
        }
    }
    return components;
}

Elements parse_elements(const pugi::xml_node& node)
{
    Elements elements;
    elements.components = parse_components(node);
    return elements;
}

System parse_system(const pugi::xml_node& node)
{
    System sys;
    sys.name = node.attribute("name").as_string();
    sys.description = node.attribute("description").as_string();

    const auto elements_node = node.child("ssd:Elements");
    sys.elements = parse_elements(elements_node);

    const auto connections_node = node.child("ssd:Connections");
    sys.connections = parse_connections(connections_node);

    return sys;
}

SystemStructureDescription parse_ssp(const fs::path& path)
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

    const auto root = doc.child("ssd:SystemStructureDescription");

    SystemStructureDescription desc;
    desc.name = root.attribute("name").as_string();
    desc.version = root.attribute("version").as_string();

    if (desc.version != "1.0") {
        throw std::runtime_error("Unsupported SSP version: '" + desc.version + "'!");
    }

    const auto system_node = root.child("ssd:System");
    desc.system = parse_system(system_node);

    return desc;
}

#endif // VICO_SSP_PARSER_HPP
