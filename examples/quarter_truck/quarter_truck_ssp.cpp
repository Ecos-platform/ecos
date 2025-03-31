#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/ssp/ssp_loader.hpp"
#include "ecos/util/plotter.hpp"

#include <spdlog/stopwatch.h>

using namespace ecos;

int main()
{
    log::set_logging_level(ecos::log::level::debug);

    const std::filesystem::path sspFolder = std::string(DATA_FOLDER) + "/ssp/quarter_truck";

    try {
        const auto ss = load_ssp(sspFolder);
        const auto sim = ss->load(std::make_unique<fixed_step_algorithm>(1.0 / 100));

        auto csvWriter = std::make_unique<csv_writer>("results/quarter_truck.csv");
        const auto outputPath = csvWriter->output_path();
        csv_config& config = csvWriter->config();
        config.register_variable("chassis::zChassis");
        config.register_variable("wheel::zWheel");
        config.register_variable("ground::zGround");
        sim->add_listener("csv_writer", std::move(csvWriter));

        spdlog::stopwatch sw;
        sim->init("initialValues");
        sim->step_until(5);
        log::info("Elapsed {:.4f}s", sw);

        sim->terminate();

        plot_csv(outputPath, sspFolder / "ChartConfig.xml");
    } catch (const std::exception& ex) {

        log::err(ex.what());
    }
}
