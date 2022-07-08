#include "vico/algorithm/fixed_step_algorithm.hpp"
#include "vico/fmi/fmi_model.hpp"
#include "vico/simulation.hpp"

#include <iostream>

using namespace vico;

int main()
{
    std::string fmuPath("../../data/fmus/2.0/20sim/ControlledTemperature.fmu");
    auto fmuModel = fmi_model(fmuPath);

    simulation sim(std::make_unique<fixed_step_algorithm>(1.0 / 100));
    sim.add_slave(fmuModel.instantiate("slave"));

    auto p = sim.get_real_property("slave::Temperature_Room");
    p->set_output_modifier([](double value) {
        return value * 10;
    });

    sim.init();
    std::cout << p->get_value() << std::endl;

    sim.step();

    std::cout << p->get_value() << std::endl;

    sim.terminate();
}
