#include <vico/algorithm/fixed_step_algorithm.hpp>
#include <vico/ssp/ssp_loader.hpp>
#include "vico/listeners/csv_writer.hpp"

#include <iostream>

using namespace vico;

namespace
{

double dummyModifier(double value)
{
    return value * 2;
}

} // namespace

int main()
{
    auto ss = load_ssp("../data/ssp/quarter_truck/quarter-truck.ssp");
    auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0 / 100), "initialValues");

    auto p = sim->get_real_property("chassis.zChassis");
    p->set_output_modifier(&dummyModifier);

    csv_config config;
    config.log_variable("chassis.zChassis");
    config.log_variable("wheel.zWheel");
    config.log_variable("ground.zGround");

    sim->add_listener(std::make_unique<csv_writer>("results/quarter_truck.csv", config));

    sim->init();
    while (sim->time() < 1) {
        sim->step();
        std::cout << p->operator()() << std::endl;
    }

    sim->terminate();
}
