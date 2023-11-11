
#ifndef ECOS_FMI_SCALAR_VARIABLE_HPP
#define ECOS_FMI_SCALAR_VARIABLE_HPP

#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace fmilibcpp
{

struct real_attributes
{
    std::optional<double> start;
};

struct integer_attributes
{
    std::optional<int> start;
};

struct string_attributes
{
    std::optional<std::string> start;
};

struct boolean_attributes
{
    std::optional<bool> start;
};

using value_ref = unsigned int;
using type_attributes = std::variant<integer_attributes, real_attributes, string_attributes, boolean_attributes>;

inline std::string type_name(const type_attributes& attribute)
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
    value_ref vr;
    std::string name;
    std::optional<std::string> causality;
    std::optional<std::string> variability;
    type_attributes typeAttributes;

    [[nodiscard]] bool is_integer() const
    {
        return typeAttributes.index() == 0;
    }

    [[nodiscard]] bool is_real() const
    {
        return typeAttributes.index() == 1;
    }

    [[nodiscard]] bool is_string() const
    {
        return typeAttributes.index() == 2;
    }

    [[nodiscard]] bool is_boolean() const
    {
        return typeAttributes.index() == 3;
    }
};

using model_variables = std::vector<scalar_variable>;

} // namespace fmilibcpp

#endif // ECOS_FMI_SCALAR_VARIABLE_HPP
