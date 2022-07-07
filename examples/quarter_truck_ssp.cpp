#include <vico/algorithm/fixed_step_algorithm.hpp>
#include <vico/ssp/ssp_loader.hpp>
#include "vico/listeners/csv_writer.hpp"

#include <iostream>

using namespace vico;

int main()
{
    auto ss = load_ssp("../data/ssp/quarter_truck");
    auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0 / 100));

    auto p = sim->get_real_property("chassis::zChassis");

    csv_config config;
    config.log_variable("chassis::zChassis");
    config.log_variable("wheel::zWheel");
    config.log_variable("ground::zGround");

    auto csvWriter = std::make_unique<csv_writer>("results/quarter_truck.csv", config);
    csvWriter->enable_plotting("../data/ssp/quarter_truck/ChartConfig.xml");
    sim->add_listener(std::move(csvWriter));

    sim->init("initialValues");
    while (sim->time() < 5) {
        sim->step();
    }
    std::cout << p->operator()() << std::endl;

    sim->terminate();
}
