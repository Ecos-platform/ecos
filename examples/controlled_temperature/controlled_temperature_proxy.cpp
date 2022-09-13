#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/logger.hpp"
#include "ecos/model_resolver.hpp"
#include "ecos/simulation.hpp"

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
        log::info("Value={}", p->get_value());
        sim.step(10);
        log::info("Value={}", p->get_value());

        sim.terminate();
    } catch (const std::exception& ex) {
        log::err(ex.what());
    }
}
