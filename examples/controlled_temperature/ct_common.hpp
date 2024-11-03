
#ifndef LIBECOS_CT_COMMON_HPP
#define LIBECOS_CT_COMMON_HPP

#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/model_resolver.hpp"
#include "ecos/simulation.hpp"


using namespace ecos;

inline void run(const std::string& fmuPath)
{
    set_logging_level(log::level::debug);

    try {
        const auto resolver = default_model_resolver();
        const auto fmuModel = resolver->resolve(fmuPath);

        simulation sim(std::make_unique<fixed_step_algorithm>(1.0 / 100));
        sim.add_slave(fmuModel->instantiate("slave"));

        const auto p = sim.get_real_property("slave::Temperature_Room");
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

#endif // LIBECOS_CT_COMMON_HPP
