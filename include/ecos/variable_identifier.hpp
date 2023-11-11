
#ifndef ECOS_VARIABLE_IDENTIFIER_HPP
#define ECOS_VARIABLE_IDENTIFIER_HPP

#include <ostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

namespace ecos
{

struct variable_identifier
{

    const std::string instanceName;
    const std::string variableName;

    variable_identifier(const char* identifier)
        : variable_identifier(parse(identifier))
    { }

    variable_identifier(std::string instanceName, std::string variableName)
        : instanceName(std::move(instanceName))
        , variableName(std::move(variableName))
    { }

    [[nodiscard]] std::string str() const
    {
        return instanceName + "::" + variableName;
    }

    bool operator==(const variable_identifier& other) const
    {
        return instanceName == other.instanceName && variableName == other.variableName;
    }

    bool operator<(const variable_identifier& other) const
    {
        return std::tie(instanceName, variableName) < std::tie(other.instanceName, other.variableName);
    }

    friend std::ostream& operator<<(std::ostream& os, const variable_identifier& v)
    {
        os << v.str();
        return os;
    }

private:
    static variable_identifier parse(const std::string& identifier)
    {
        auto pos = identifier.find("::");
        if (pos == std::string::npos) {
            throw std::runtime_error("Error parsing variable identifier. A '::' must be present!");
        }

        return {identifier.substr(0, pos), identifier.substr(pos + 2)};
    }
};

} // namespace ecos

#endif // ECOS_VARIABLE_IDENTIFIER_HPP
