
#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/model_resolver.hpp"
#include "ecos/simulation.hpp"

#include <iostream>

using namespace ecos;

int main()
{
    spdlog::set_level(spdlog::level::debug);

    std::string fmuPath("../../data/fmus/2.0/20sim/ControlledTemperature.fmu");
    auto resolver = default_model_resolver();
    auto fmuModel = resolver->resolve(fmuPath);

    simulation sim(std::make_unique<fixed_step_algorithm>(1.0 / 100));
    sim.add_slave(fmuModel->instantiate("slave"));

    auto p = sim.get_real_property("slave::Temperature_Room");
    p->set_output_modifier([](double value) {
        return value * 10;
    });

    sim.init();
    std::cout << p->get_value() << std::endl;
    sim.step(10);
    std::cout << p->get_value() << std::endl;

    sim.reset();
    sim.init();
    std::cout << p->get_value() << std::endl;
    sim.step(10);
    std::cout << p->get_value() << std::endl;

    sim.terminate();
}
