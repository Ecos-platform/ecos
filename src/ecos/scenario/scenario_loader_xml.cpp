
#include "ecos/logger/logger.hpp"
#include "ecos/scenario/scenario.hpp"
#include "ecos/scenario/scenario_loader.hpp"

#include <pugixml.hpp>

using namespace ecos;

void ecos::load_scenario(simulation& sim, const std::filesystem::path& config)
{

    if (!exists(config)) {
        throw std::runtime_error("No such file: " + absolute(config).string());
    }

    if (const auto ext = config.extension().string(); ext != ".xml") {
        throw std::runtime_error("Wrong config extension. Was " + ext + ", expected " + ".xml");
    }

    pugi::xml_document doc;
    if (pugi::xml_parse_result result = doc.load_file(config.c_str()); !result) {
        throw std::runtime_error("Unable to parse '" + absolute(config).string() + "': " + result.description());
    }

    size_t numActions{};
    const auto root = doc.child("ecos:Scenario");
    for (const auto& action : root) {
        ++numActions;
        const auto t = action.attribute("t").as_double();
        const auto epsAttr = action.attribute("eps");
        std::optional<double> eps;
        if (epsAttr) eps = epsAttr.as_double();

        for (const auto& variable : action) {

            const variable_identifier id = variable.attribute("id").as_string();

            pugi::xml_node var;
            if ((var = variable.child("ecos:real"))) {
                auto p = sim.get_real_property(id);
                if (!p) {
                    log::warn("No variable with id: {}", id.str());
                    continue;
                }
                const double value = var.attribute("value").as_double();
                sim.invoke_at(
                    t, [p, value] {
                        p->set_value(value);
                    },
                    eps);
            } else if ((var = variable.child("ecos:integer"))) {
                auto p = sim.get_int_property(id);
                if (!p) {
                    log::warn("No variable with id: {}", id.str());
                    continue;
                }
                const int value = var.attribute("value").as_int();
                sim.invoke_at(
                    t, [p, value] {
                        p->set_value(value);
                    },
                    eps);
            } else if ((var = variable.child("ecos:boolean"))) {
                auto p = sim.get_bool_property(id);
                if (!p) {
                    log::warn("No variable with id: {}", id.str());
                    continue;
                }
                const bool value = var.attribute("value").as_bool();
                sim.invoke_at(
                    t, [p, value] {
                        p->set_value(value);
                    },
                    eps);
            } else if ((var = variable.child("ecos:string"))) {
                auto p = sim.get_string_property(id);
                if (!p) {
                    log::warn("No variable with id: {}", id.str());
                    continue;
                }
                const std::string value = var.attribute("value").as_string();
                sim.invoke_at(
                    t, [p, value] {
                        p->set_value(value);
                    },
                    eps);
            } else {
                throw std::runtime_error("Assertion error");
            }
        }
    }

    log::debug("Applied scenario {} with {} actions to simulation", config.string(), numActions);
}
