#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/logger.hpp"
#include "ecos/scenario/scenario_loader.hpp"
#include "ecos/ssp/ssp_loader.hpp"

#include <filesystem>
#include <spdlog/stopwatch.h>

using namespace ecos;

int main()
{
    log::set_logging_level(ecos::log::level::debug);

    const auto sspFile = std::string(SOURCE_DIR) + "/gunnerus-trajectory.ssp";
    if (!std::filesystem::exists(sspFile)) {
        log::err("gunnerus-trajectory.ssp has not been generated yet. Run sspgen.");
        return 1;
    }

    try {
        auto ss = load_ssp(sspFile);
        auto sim = ss->load(std::make_unique<fixed_step_algorithm>(0.05));

        csv_config config;
        config.log_variable("vesselModel::cgShipMotion.nedDisplacement.north");
        config.log_variable("vesselModel::cgShipMotion.nedDisplacement.east");
        config.log_variable("vesselModel::cgShipMotion.nedDisplacement.down");

        auto csvWriter = std::make_unique<csv_writer>("results/gunnerus_trajectory.csv", config);
        csvWriter->enable_plotting(std::string(SOURCE_DIR) + "/ChartConfig.xml");
        sim->add_listener(std::move(csvWriter));

        load_scenario(*sim, std::string(SOURCE_DIR) + "/scenario.xml");

        spdlog::stopwatch sw;
        sim->init("initialValues");
        sim->step_until(250);
        log::info("Elapsed {:.3}s", sw);
        sim->terminate();
    } catch (const std::exception& ex) {
        log::err(ex.what());
    }
}
