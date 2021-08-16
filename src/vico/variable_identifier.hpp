
#ifndef VICO_VARIABLE_IDENTIFIER_HPP
#define VICO_VARIABLE_IDENTIFIER_HPP

#include <string>
#include <utility>

namespace vico
{

struct variable_identifier
{

    const std::string instanceName;
    const std::string variableName;

    explicit variable_identifier(const std::string &identifier);

private:
    explicit variable_identifier(const std::pair<std::string, std::string> &identifier);
};

} // namespace vico

#endif // VICO_VARIABLE_IDENTIFIER_HPP
