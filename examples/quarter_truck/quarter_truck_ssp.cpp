#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/ssp/ssp_loader.hpp"

#include <spdlog/stopwatch.h>

using namespace ecos;

int main()
{
    log::set_logging_level(ecos::log::level::debug);

    try {
        auto ss = load_ssp(std::string(DATA_FOLDER) + "/ssp/quarter_truck");
        auto sim = ss->load(std::make_unique<fixed_step_algorithm>(1.0 / 100));

        auto csvWriter = std::make_unique<csv_writer>("results/quarter_truck.csv");
        csv_config& config = csvWriter->config();
        config.register_variable("chassis::zChassis");
        config.register_variable("wheel::zWheel");
        config.register_variable("ground::zGround");
        config.enable_plotting(std::string(DATA_FOLDER) + "/ssp/quarter_truck/ChartConfig.xml");
        sim->add_listener("csv_writer", std::move(csvWriter));

        spdlog::stopwatch sw;
        sim->init("initialValues");
        sim->step_until(5);
        log::info("Elapsed {:.4f}s", sw);

        sim->terminate();
    } catch (const std::exception& ex) {
        log::err(ex.what());
    }
}
