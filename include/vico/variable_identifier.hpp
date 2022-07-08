
#ifndef VICO_VARIABLE_IDENTIFIER_HPP
#define VICO_VARIABLE_IDENTIFIER_HPP

#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace vico
{

struct variable_identifier
{

    const std::string instanceName;
    const std::string variableName;

    variable_identifier(const std::string& identifier)
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

    bool operator!=(const variable_identifier& other) const
    {
        return !(*this == other);
    }

    bool operator<(const variable_identifier& other) const
    {
        return instanceName < other.instanceName || (instanceName == other.instanceName && variableName < other.variableName);
    }

    std::ostream& operator<<(std::ostream& os) const
    {
        os << str();
        return os;
    }

private:
    static variable_identifier parse(const std::string& identifier)
    {
        auto result = identifier.find("::");
        if (result == std::string::npos) {
            throw std::runtime_error("Error parsing variable identifier. A '::' must be present!");
        }

        return {identifier.substr(0, result), identifier.substr(result + 2)};
    }
};

} // namespace vico

#endif // VICO_VARIABLE_IDENTIFIER_HPP
