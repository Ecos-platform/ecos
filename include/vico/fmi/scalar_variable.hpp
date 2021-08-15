
#ifndef VICO_SCALAR_VARIABLE_HPP
#define VICO_SCALAR_VARIABLE_HPP

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace vico {

    struct real {
        std::optional<double> start;
    };

    struct integer {
        std::optional<int> start;
    };

    struct string {
        std::optional<std::string> start;
    };

    struct boolean {
        std::optional<bool> start;
    };

    using type_attribute = std::variant<real, integer, string, boolean>;

    struct scalar_variable {
        unsigned int vr;
        std::string name;
        std::optional<std::string> causality;
        std::optional<std::string> variability;
        type_attribute typeAttribute;

        [[nodiscard]] bool is_real() const {
            return typeAttribute.index() == 0;
        }

        [[nodiscard]] bool is_integer() const {
            return typeAttribute.index() == 1;
        }

        [[nodiscard]] bool is_string() const {
            return typeAttribute.index() == 2;
        }

        [[nodiscard]] bool is_boolean() const {
            return typeAttribute.index() == 3;
        }
    };

    using model_variables = std::vector<scalar_variable>;

}// namespace vico

#endif // VICO_SCALAR_VARIABLE_HPP
