
#ifndef VICO_VARIABLE_IDENTIFIER_HPP
#define VICO_VARIABLE_IDENTIFIER_HPP

namespace vico
{

struct variable_identifier
{

    const std::string instanceName;
    const std::string variableName;

    static variable_identifier parse(const std::string& str)
    {

        auto result = str.find('.');
        if (result == std::string::npos) {

            throw std::runtime_error("Error parsing variable identifier. A '.' must be present!");
        }

        return {str.substr(0, result), str.substr(result+1)};
    }
};

} // namespace vico

#endif // VICO_VARIABLE_IDENTIFIER_HPP
