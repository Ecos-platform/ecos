
#ifndef LIBECOS_PARSE_SCENARIO_NODE_HPP
#define LIBECOS_PARSE_SCENARIO_NODE_HPP

#include "ecos/scenario.hpp"

#include <memory>
#include <pugixml.hpp>


inline std::unique_ptr<ecos::scenario> parse_scenario_node(const pugi::xml_node& root, size_t& numActions)
{
    std::unique_ptr<ecos::scenario> parsed_scenario;

    if (const auto nameAttr = root.attribute("name")) {
        parsed_scenario->name = nameAttr.as_string();
    }

    std::optional<double> eps;
    const auto epsAttr = root.attribute("eps");
    if (epsAttr) eps = epsAttr.as_double();

    for (const auto& action : root) {
        ++numActions;
        const auto t = action.attribute("t").as_double();

        const auto epsSubAttr = action.attribute("eps");
        std::optional<double> subEps = eps;
        if (epsSubAttr) subEps = epsSubAttr.as_double();

        for (const auto& variable : action) {

            const ecos::variable_identifier id = variable.attribute("id").as_string();

            pugi::xml_node var;
            if ((var = variable.child("ecos:real"))) {
                const double value = var.attribute("value").as_double();
                parsed_scenario->add_action<double>(t, id, value);

            } else if ((var = variable.child("ecos:integer"))) {
                const int value = var.attribute("value").as_int();
                parsed_scenario->add_action<int>(t, id, value);

            } else if ((var = variable.child("ecos:boolean"))) {
                const bool value = var.attribute("value").as_bool();
                parsed_scenario->add_action<bool>(t, id, value);

            } else if ((var = variable.child("ecos:string"))) {
                const std::string value = var.attribute("value").as_string();
                parsed_scenario->add_action<std::string>(t, id, value);
            } else {
                throw std::runtime_error("Assertion error");
            }
        }
    }

    return parsed_scenario;
}

#endif // LIBECOS_PARSE_SCENARIO_NODE_HPP
