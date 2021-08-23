
#ifndef VICO_SCALAR_VARIABLE_HPP
#define VICO_SCALAR_VARIABLE_HPP

#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace vico
{

struct real
{
    std::optional<double> start;
};

struct integer
{
    std::optional<int> start;
};

struct string
{
    std::optional<std::string> start;
};

struct boolean
{
    std::optional<bool> start;
};

using value_ref = unsigned int;
using type_attribute = std::variant<integer, real, string, boolean>;

inline std::string type_name(const type_attribute& attribute)
{
    switch (attribute.index()) {
        case 0: return "integer";
        case 1: return "real";
        case 2: return "string";
        case 3: return "boolean";
        default: throw std::runtime_error("Invalid variant");
    }
}

struct scalar_variable
{

    value_ref valueRef;
    std::string name;
    std::optional<std::string> causality;
    std::optional<std::string> variability;
    type_attribute typeAttribute;

    [[nodiscard]] bool is_real() const
    {
        return typeAttribute.index() == 0;
    }

    [[nodiscard]] bool is_integer() const
    {
        return typeAttribute.index() == 1;
    }

    [[nodiscard]] bool is_string() const
    {
        return typeAttribute.index() == 2;
    }

    [[nodiscard]] bool is_boolean() const
    {
        return typeAttribute.index() == 3;
    }
};

using model_variables = std::vector<scalar_variable>;

} // namespace vico

#endif // VICO_SCALAR_VARIABLE_HPP
