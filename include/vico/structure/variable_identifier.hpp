
#ifndef VICO_VARIABLE_IDENTIFIER_HPP
#define VICO_VARIABLE_IDENTIFIER_HPP

#include <stdexcept>
#include <string>
#include <utility>
#include <ostream>

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

    [[nodiscard]] std::string str() const {
        return instanceName + "::" + variableName;
    }

    std::ostream& operator<<(std::ostream& os) const
    {
        os << str();
        return os;
    }

private:
    explicit variable_identifier(const std::pair<std::string, std::string>& identifier)
        : instanceName(identifier.first)
        , variableName(identifier.second)
    { }

    static std::pair<std::string, std::string> parse(const std::string& identifier)
    {
        auto result = identifier.find('.');
        if (result == std::string::npos) {
            throw std::runtime_error("Error parsing variable identifier. A '.' must be present!");
        }

        return {identifier.substr(0, result), identifier.substr(result + 1)};
    }
};

} // namespace vico

#endif // VICO_VARIABLE_IDENTIFIER_HPP
