#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/scenario/scenario_loader.hpp"
#include "ecos/ssp/ssp_loader.hpp"

#include <filesystem>

using namespace ecos;

int main()
{
    set_logging_level(log::level::debug);

    std::filesystem::path ssp_dir = std::string(DATA_FOLDER) + "/ssp/dp_ship";

    try {
        const auto ss = load_ssp(ssp_dir);
        const auto sim = ss->load(std::make_unique<fixed_step_algorithm>(0.04));

        load_scenario(*sim, ssp_dir / "waypoints_scenario.xml");

        const auto writer = std::make_shared<csv_writer>("results/dp_ship_cpp.csv");
        writer->config().load(ssp_dir / "LogConfig.xml");
        writer->config().enable_plotting(ssp_dir / "ChartConfig.xml");
        sim->add_listener("writer", writer);

        sim->init();
        sim->step_until(1500);

        sim->terminate();
    } catch (const std::exception& ex) {

        log::err(ex.what());
    }
}
