
#include "ecos/scenario.hpp"
#include "ecos/scenario_loader.hpp"
#include "ecos/variable_identifier.hpp"

#include <pugixml.hpp>

using namespace ecos;

void ecos::load_configuration(const std::filesystem::path& config)
{

    if (!std::filesystem::exists(config)) {
        throw std::runtime_error("No such file: " + std::filesystem::absolute(config).string());
    }

    const auto ext = config.extension().string();
    if (ext != ".xml") {
        throw std::runtime_error("Wrong config extension. Was " + ext + ", expected " + ".xml");
    }

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(config.c_str());
    if (!result) {
        throw std::runtime_error("Unable to parse '" +
            std::filesystem::absolute(config).string() +
            "': " + result.description());
    }

    const auto root = doc.child("Scenario");
    for (const auto& action : root.child("actions")) {
        const auto t = action.attribute("t").as_double();
        const auto variable = action.child("variable");
        const variable_identifier id = variable.attribute("id").as_string();

        pugi::xml_node var;
        if ((var = variable.child("real"))) {

        } else if ((var = variable.child("integer"))) {

        } else if ((var = variable.child("boolean"))) {

        } else if ((var = variable.child("string"))) {

        } else {
            throw std::runtime_error("Assertion error");
        }
    }
}
