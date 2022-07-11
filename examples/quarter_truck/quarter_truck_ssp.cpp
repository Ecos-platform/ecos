#include "vico/algorithm/fixed_step_algorithm.hpp"
#include "vico/listeners/csv_writer.hpp"
#include "vico/ssp/ssp_loader.hpp"

#include <iostream>
#include <spdlog/stopwatch.h>

using namespace vico;

int main()
{
    spdlog::set_level(spdlog::level::debug);

    auto ss = load_ssp("../../data/ssp/quarter_truck");
    auto sim = ss->load(std::make_unique<fixed_step_algorithm>(1.0 / 100));

    csv_config config;
    config.log_variable("chassis::zChassis");
    config.log_variable("wheel::zWheel");
    config.log_variable("ground::zGround");

    auto csvWriter = std::make_unique<csv_writer>("results/quarter_truck.csv", config);
    csvWriter->enable_plotting("../../data/ssp/quarter_truck/ChartConfig.xml");
    sim->add_listener(std::move(csvWriter));

    spdlog::stopwatch sw;
    sim->init("initialValues");
    sim->step_until(5);
    spdlog::info("Elapsed {}", sw);

    sim->terminate();
}
