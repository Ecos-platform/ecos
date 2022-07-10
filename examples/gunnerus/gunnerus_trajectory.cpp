#include "vico/algorithm/fixed_step_algorithm.hpp"
#include "vico/listeners/csv_writer.hpp"
#include "vico/ssp/ssp_loader.hpp"

#include <iostream>
#include <filesystem>

#include <spdlog/spdlog.h>

using namespace vico;

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

    bool reset = false;
    auto resetProperty = sim->get_bool_property("vesselModel::reset_position");
    auto trackControllerProperty = sim->get_bool_property("trackController::enable");

    sim->init("initialValues");
    while (sim->time() < 250) {
        if (sim->time() > 50 && !reset) {
            resetProperty->set_value(true);
            sim->step();
            resetProperty->set_value(false);
            trackControllerProperty->set_value(true);
            reset = true;
        } else {
            sim->step();
        }
    }

    sim->terminate();
}
