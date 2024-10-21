
#include "ssp.hpp"

#include "ecos/util/temp_dir.hpp"
#include "ecos/util/unzipper.hpp"

#include <ranges>


namespace ecos::ssp
{

Connection parse_connection(
    const pugi::xml_node& node,
    const std::unordered_map<std::string, Component>& components)
{
    const std::string startElement = node.attribute("startElement").as_string();
    const std::string startConnector = node.attribute("startConnector").as_string();
    const std::string endElement = node.attribute("endElement").as_string();
    const std::string endConnector = node.attribute("endConnector").as_string();

    if (components.contains(startElement)) {
        const Component& c = components.at(startElement);
        if (!c.connectors.contains(startConnector)) {
            throw std::runtime_error("No connector named: '" + startConnector + "' defined for element: '" + startElement + "'!");
        }
    } else {
        throw std::runtime_error("No element named: " + startElement);
    }
    if (components.contains(endElement)) {
        const Component& c = components.at(startElement);
        if (!c.connectors.contains(startConnector)) {
            throw std::runtime_error("No connector named: '" + endConnector + "' defined for element: '" + endElement + "'!");
        }
    } else {
        throw std::runtime_error("No element named: " + startElement);
    }

    Connection c = {startElement, startConnector, endElement, endConnector};
    if (const auto transformationNode = node.child("ssc:LinearTransformation")) {
        const double factor = transformationNode.attribute("factor").as_double();
        const double offset = transformationNode.attribute("offset").as_double();
        c.linearTransformation = {factor, offset};
    }
    return c;
}

std::vector<Connection> parse_connections(
    const pugi::xml_node& node,
    const std::unordered_map<std::string, Component>& components)
{
    std::vector<Connection> connections;
    for (const auto c : node) {
        connections.emplace_back(parse_connection(c, components));
    }
    return connections;
}

Connector parse_connector(const pugi::xml_node& node)
{
    const std::string connectorName = node.attribute("name").as_string();
    const std::string connectorKind = node.attribute("kind").as_string();
    Connector connector = {connectorName, connectorKind};
    if (node.child("ssc:Real")) {
        connector.type.value = 0.;
    } else if (node.child("ssc:Integer")) {
        connector.type.value = 0;
    } else if (node.child("ssc:Boolean")) {
        connector.type.value = false;
    } else if (node.child("ssc:String")) {
        connector.type.value = "";
    }
    return connector;
}

std::unordered_map<std::string, Connector> parse_connectors(const pugi::xml_node& node)
{
    std::unordered_map<std::string, Connector> connectors;
    for (const auto c : node) {
        const auto connector = parse_connector(c);
        connectors.emplace(connector.name, connector);
    }
    return connectors;
}

Parameter parse_parameter(const pugi::xml_node& node)
{
    const auto name = node.attribute("name").as_string();
    Parameter parameter{name};
    pugi::xml_node typeNode;
    if (node.child("ssv:Real")) {
        typeNode = node.child("ssv:Real");
        const double value = typeNode.attribute("value").as_double();
        parameter.type.value = value;
    } else if (node.child("ssv:Integer")) {
        typeNode = node.child("ssv:Integer");
        const int value = typeNode.attribute("value").as_int();
        parameter.type.value = value;
    } else if (node.child("ssv:Boolean")) {
        typeNode = node.child("ssv:Boolean");
        const bool value = typeNode.attribute("value").as_bool();
        parameter.type.value = value;
    } else if (node.child("ssv:String")) {
        typeNode = node.child("ssv:String");
        const std::string value = typeNode.attribute("value").as_string();
        parameter.type.value = value;
    } else {
        throw std::runtime_error("Unknown XML node in ssv:Parameter encountered!");
    }
    if (const auto unit = typeNode.attribute("unit"); !unit.empty()) {
        parameter.type.unit = unit.as_string();
    }
    return parameter;
}

std::unordered_map<std::string, ParameterSet>
parse_parameter_bindings(const std::filesystem::path& dir, const pugi::xml_node& node)
{
    std::unordered_map<std::string, ParameterSet> parameterSets;
    for (const auto parameterBindingNode : node) {
        pugi::xml_node parameterSetNode;
        std::unique_ptr<pugi::xml_document> doc;
        if (const auto parameterValues = parameterBindingNode.child("ssd:ParameterValues")) {
            parameterSetNode = parameterValues.child("ssv:ParameterSet");
        } else {
            const auto source = parameterBindingNode.attribute("source").as_string();
            doc = std::make_unique<pugi::xml_document>();
            if (pugi::xml_parse_result result = doc->load_file(std::filesystem::path(dir / source).c_str()); !result) {
                throw std::runtime_error(
                    "Unable to parse '" + absolute(std::filesystem::path(dir / source)).string() + "': " + result.description());
            }
            parameterSetNode = doc->child("ssv:ParameterSet");
        }
        const auto name = parameterSetNode.attribute("name").as_string();
        ParameterSet set{name};
        const auto parametersNode = parameterSetNode.child("ssv:Parameters");
        for (const auto parameterNode : parametersNode) {
            Parameter p = parse_parameter(parameterNode);
            set.parameters.emplace_back(p);
        }
        parameterSets[name] = set;
    }
    return parameterSets;
}

Component parse_component(const std::filesystem::path& dir, const pugi::xml_node& node)
{
    const std::string componentName = node.attribute("name").as_string();
    const std::string componentSource = node.attribute("source").as_string();
    const auto connectors = parse_connectors(node.child("ssd:Connectors"));
    const auto parameterSets = parse_parameter_bindings(dir, node.child("ssd:ParameterBindings"));
    return {componentName, componentSource, connectors, parameterSets};
}

std::unordered_map<std::string, Component> parse_components(const std::filesystem::path& dir, const pugi::xml_node& node)
{
    std::unordered_map<std::string, Component> components;
    for (const auto childNode : node) {
        if (std::string(childNode.name()) == "ssd:Component") {
            Component c = parse_component(dir, childNode);
            components[c.name] = c;
        }
    }
    return components;
}

Elements parse_elements(const std::filesystem::path& dir, const pugi::xml_node& node)
{
    Elements elements;
    elements.components = parse_components(dir, node);

    // collect parameterSets by name
    for (const auto& component : elements.components | std::views::values) {
        for (const auto& [parameterSetName, parameterSet] : component.parameterSets) {
            auto& list = elements.parameterSets[parameterSetName][component];
            list.insert(list.end(), parameterSet.parameters.begin(), parameterSet.parameters.end());
        }
    }

    return elements;
}

std::vector<Annotation> parse_annotations(const pugi::xml_node& node)
{
    std::vector<Annotation> annotations;
    for (const auto& annotationNode : node) {
        const auto type = annotationNode.attribute("type").as_string();
        annotations.emplace_back(Annotation{type, annotationNode});
    }
    return annotations;
}

DefaultExperiment parse_default_experiment(const pugi::xml_node& node)
{
    const auto start = node.attribute("startTime");
    const auto stop = node.attribute("stopTime");
    DefaultExperiment ex;
    if (start) {
        ex.start = start.as_double();
    }
    if (stop) {
        ex.stop = stop.as_double();
    }
    if (const auto annotationsNode = node.child("ssd:Annotations")) {
        ex.annotations = parse_annotations(annotationsNode);
    }
    return ex;
}

System parse_system(const std::filesystem::path& dir, const pugi::xml_node& node)
{
    System sys;
    sys.name = node.attribute("name").as_string();
    sys.description = node.attribute("description").as_string();

    const auto elementsNode = node.child("ssd:Elements");
    sys.elements = parse_elements(dir, elementsNode);

    const auto connectionsNode = node.child("ssd:Connections");
    sys.connections = parse_connections(connectionsNode, sys.elements.components);

    return sys;
}

struct SystemStructureDescription::Impl
{

    std::string name;
    std::string version;

    System system;
    std::optional<DefaultExperiment> defaultExperiment;

    std::filesystem::path dir_;
    pugi::xml_document doc_;
    std::shared_ptr<temp_dir> tmp_ = nullptr;

    explicit Impl(const std::filesystem::path& path)
    {

        if (!exists(path)) {
            throw std::runtime_error("No such file: " + absolute(path).string());
        }

        if (is_directory(path)) {
            dir_ = path;
        } else {
            tmp_ = std::make_unique<temp_dir>("ssp");
            dir_ = tmp_->path();
            if (!unzip(path, dir_)) {
                throw std::runtime_error("Failed to unzip contents..");
            }
        }

        pugi::xml_parse_result result = doc_.load_file(std::filesystem::path(dir_ / "SystemStructure.ssd").c_str());
        if (!result) {
            throw std::runtime_error(
                "Unable to parse '" + absolute(std::filesystem::path(dir_ / "SystemStructure.ssd")).string() + "': " +
                result.description());
        }

        const auto root = doc_.child("ssd:SystemStructureDescription");

        name = root.attribute("name").as_string();
        version = root.attribute("version").as_string();

        if (version != "1.0") {
            throw std::runtime_error("Unsupported SSP version: '" + version + "'!");
        }

        const auto systemNode = root.child("ssd:System");
        system = parse_system(dir_, systemNode);

        if (const auto defaultNode = root.child("ssd:DefaultExperiment")) {
            defaultExperiment = parse_default_experiment(defaultNode);
        }
    }

    [[nodiscard]] std::filesystem::path file(const std::filesystem::path& source) const
    {
        return dir_ / source;
    }

    ~Impl() = default;
};

SystemStructureDescription::SystemStructureDescription(const std::filesystem::path& path)
    : pimpl_(std::make_unique<Impl>(path))
    , name(pimpl_->name)
    , version(pimpl_->version)
    , system(pimpl_->system)
    , defaultExperiment(pimpl_->defaultExperiment)
{ }

std::filesystem::path SystemStructureDescription::file(const std::filesystem::path& source) const
{
    return pimpl_->file(source);
}

std::filesystem::path SystemStructureDescription::dir() const
{
    return pimpl_->dir_;
}

std::shared_ptr<temp_dir> SystemStructureDescription::get_temp_dir() const
{
    return pimpl_->tmp_;
}

SystemStructureDescription::~SystemStructureDescription() = default;

} // namespace ecos::ssp