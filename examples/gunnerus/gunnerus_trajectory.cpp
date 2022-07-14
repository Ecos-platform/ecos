#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/ssp/ssp_loader.hpp"

#include <filesystem>
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>

using namespace ecos;

int main()
{
    spdlog::set_level(spdlog::level::debug);

    const auto sspFile = "../../data/ssp/gunnerus/gunnerus-trajectory.ssp";
    if (!std::filesystem::exists(sspFile)) {
        std::cerr << "gunnerus-trajectory.ssp has not been generated yet. Run sspgen." << std::endl;
    }

    auto ss = load_ssp(sspFile);
    auto sim = ss->load(std::make_unique<fixed_step_algorithm>(0.05));

    csv_config config;
    config.log_variable("vesselModel::cgShipMotion.nedDisplacement.north");
    config.log_variable("vesselModel::cgShipMotion.nedDisplacement.east");
    config.log_variable("vesselModel::cgShipMotion.nedDisplacement.down");

    auto csvWriter = std::make_unique<csv_writer>("results/gunnerus.csv", config);
    csvWriter->enable_plotting("../../data/ssp/gunnerus/ChartConfig.xml");
    sim->add_listener(std::move(csvWriter));

    auto resetProperty = sim->get_bool_property("vesselModel::reset_position");
    auto trackControllerProperty = sim->get_bool_property("trackController::enable");


    sim->invoke_at([&sim, &resetProperty, &trackControllerProperty]{
        resetProperty->set_value(true);

        sim->invoke_at([&resetProperty, &trackControllerProperty]{
            resetProperty->set_value(false);
            trackControllerProperty->set_value(true);
        }, 50);

    }, 50, 0.01);


    spdlog::stopwatch sw;
    sim->init("initialValues");
    sim->step_until(250);
    spdlog::info("Elapsed {}", sw);
    sim->terminate();

}
