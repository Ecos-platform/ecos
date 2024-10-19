
#ifndef SMD_COMMON_HPP
#define SMD_COMMON_HPP

#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/model_resolver.hpp"
#include "ecos/simulation.hpp"
#include "ecos/structure/simulation_structure.hpp"
#include <ecos/listeners/csv_writer.hpp>

using namespace ecos;

inline void run(simulation_structure& ss)
{
    set_logging_level(log::level::debug);

    try {

        auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0 / 100));
        auto csvWriter = std::make_unique<csv_writer>("results/mass_spring_damper.csv");
        sim->add_listener("csv_writer", std::move(csvWriter));

        sim->init("initialValues");
        sim->step_until(5);
        sim->terminate();
    } catch (const std::exception& ex) {

        log::err(ex.what());
    }
}

#endif //SMD_COMMON_HPP
