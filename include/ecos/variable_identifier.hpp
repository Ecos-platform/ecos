
#ifndef ECOS_VARIABLE_IDENTIFIER_HPP
#define ECOS_VARIABLE_IDENTIFIER_HPP

#include <ostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

namespace ecos
{

/* *
 * \brief Represents a variable identifier in the format "instanceName::variableName".
 *
 * A variable_identifier is used to uniquely identify a variable within a simulation.
 */
struct variable_identifier
{

    const std::string instanceName;
    const std::string variableName;

    // Constructs a variable_identifier from a string in the format "instanceName::variableName".
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

    [[nodiscard]] bool matches(const variable_identifier& pattern) const
    {
        return wildcard_match(instanceName, pattern.instanceName) &&
            wildcard_match(variableName, pattern.variableName);
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
        const auto pos = identifier.find("::");
        if (pos == std::string::npos) {
            throw std::runtime_error("Error parsing variable identifier. A '::' must be present!");
        }

        return {identifier.substr(0, pos), identifier.substr(pos + 2)};
    }

    static bool wildcard_match(const std::string& str, const std::string& pattern)
    {
        size_t s = 0, p = 0, star = std::string::npos, ss = 0;

        while (s < str.size()) {
            if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == str[s])) {
                ++s;
                ++p;
            } else if (p < pattern.size() && pattern[p] == '*') {
                star = p++;
                ss = s;
            } else if (star != std::string::npos) {
                p = star + 1;
                s = ++ss;
            } else {
                return false;
            }
        }

        while (p < pattern.size() && pattern[p] == '*') ++p;

        return p == pattern.size();
    }
};

} // namespace ecos

#endif // ECOS_VARIABLE_IDENTIFIER_HPP
