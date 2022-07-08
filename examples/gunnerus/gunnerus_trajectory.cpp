#include "vico/algorithm/fixed_step_algorithm.hpp"
#include "vico/listeners/csv_writer.hpp"
#include "vico/ssp/ssp_loader.hpp"

#include <iostream>
#include <filesystem>

using namespace vico;

int main()
{

    const auto sspFile = "../../data/ssp/gunnerus/gunnerus-trajectory.ssp";
    if (!std::filesystem::exists(sspFile)) {
        std::cerr << "gunnerus-trajectory.ssp has not been generated yet. Run sspgen." << std::endl;
    }

    auto ss = load_ssp(sspFile);
    auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0 / 100));

    csv_config config;
    config.log_variable("vesselModel::cgShipMotion.nedDisplacement.north");
    config.log_variable("vesselModel::cgShipMotion.nedDisplacement.east");
    config.log_variable("vesselModel::cgShipMotion.nedDisplacement.down");

    auto csvWriter = std::make_unique<csv_writer>("results/gunnerus.csv", config);
    csvWriter->enable_plotting("../../data/ssp/gunnerus/ChartConfig.xml");
    sim->add_listener(std::move(csvWriter));

    sim->init("initialValues");
    while (sim->time() < 5) {
        sim->step();
    }

    sim->terminate();
}
