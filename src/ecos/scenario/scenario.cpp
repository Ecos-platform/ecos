
#include "ecos/scenario.hpp"

#include "ecos/logger/logger.hpp"
#include "ecos/scenario/parse_scenario_node.hpp"

using namespace ecos;

void scenario::pre_step(simulation& sim)
{

    const double t = sim.time();

    while (!actions_.empty()) {
        auto& action = actions_.back();
        if (t >= action->timePoint) {
            log::debug("Applying scenario action at t={} for variable {}", t, action->id.str());
            action->apply(sim);
            used_actions_.emplace_back(std::move(action));
            actions_.pop_back();
        } else {
            break;
        }
    }
}
void scenario::on_reset()
{
    for (auto& action : used_actions_) {
        actions_.emplace_back(std::move(action));
    }
    used_actions_.clear();
    sortActions();
}

std::unique_ptr<scenario> scenario::load(const std::filesystem::path& config)
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
    auto parsed_scenario = parse_scenario_node(root, numActions);

    log::debug("Applied scenario '{}' with {} actions to simulation", parsed_scenario->name, numActions);

    return parsed_scenario;
}

void scenario::sortActions()
{
    std::ranges::sort(actions_, [](const auto& a, const auto& b) {
        if (!a) return false;
        if (!b) return true;
        return *a < *b;
    });
}
