
#ifndef SSP_SSP_HPP
#define SSP_SSP_HPP

#include "ecos/util/temp_dir.hpp"

#include <map>
#include <memory>
#include <optional>
#include <pugixml.hpp>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>


namespace ecos::ssp
{

struct Type
{
    std::optional<std::string> unit;
    std::variant<double, int, bool, std::string> value;

    [[nodiscard]] std::string typeName() const
    {
        if (value.index() == 0) {
            return "Real";
        } else if (value.index() == 1) {
            return "Integer";
        } else if (value.index() == 2) {
            return "Boolean";
        } else if (value.index() == 3) {
            return "String";
        } else {
            throw std::runtime_error("Invalid type!");
        }
    }

    bool operator==(const Type& other) const
    {
        return value.index() == other.value.index();
    }
};

struct Parameter
{
    std::string name;
    Type type;
};

struct ParameterSet
{
    std::string name;
    std::vector<Parameter> parameters;
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
    std::unordered_map<std::string, Connector> connectors;
    std::unordered_map<std::string, ParameterSet> parameterSets;

    bool operator<(const Component& other) const
    {
        return name < other.name;
    }
};


struct Elements
{
    std::unordered_map<std::string, Component> components;
    std::unordered_map<std::string, std::map<Component, std::vector<Parameter>>> parameterSets;
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

struct Annotation
{
    std::string type;
    pugi::xml_node node;
};

struct DefaultExperiment
{
    std::optional<double> start;
    std::optional<double> stop;

    std::vector<Annotation> annotations;
};

struct System
{
    std::string name;
    std::string description;

    Elements elements;
    std::vector<Connection> connections;
};

class SystemStructureDescription
{

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;

public:
    const std::string& name;
    const std::string& version;

    const System& system;
    const std::optional<DefaultExperiment>& defaultExperiment;

    explicit SystemStructureDescription(const std::filesystem::path& path);

    [[nodiscard]] std::filesystem::path dir() const;

    [[nodiscard]] std::filesystem::path file(const std::filesystem::path& source) const;

    [[nodiscard]] std::shared_ptr<temp_dir> get_temp_dir() const;

    ~SystemStructureDescription();
};

} // namespace ecos::ssp


#endif // SSP_SSP_HPP
