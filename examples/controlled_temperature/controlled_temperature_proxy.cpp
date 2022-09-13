#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/model_resolver.hpp"
#include "ecos/simulation.hpp"
#include "ecos/logger.hpp"

#include <iostream>

using namespace ecos;

int main()
{
    set_logging_level(ecos::log::level::info);
    std::string fmuPath = "proxyfmu://localhost?file=" + std::string(DATA_FOLDER) + "/fmus/2.0/20sim/ControlledTemperature.fmu";

    try {
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

        sim.terminate();
    } catch (std::exception& ex) {
        std::cerr << "[ERROR]: " << ex.what() << std::endl;
    }
}
