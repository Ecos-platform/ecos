
#ifndef SMD_COMMON_HPP
#define SMD_COMMON_HPP

#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/model_resolver.hpp"
#include "ecos/simulation.hpp"
#include "ecos/structure/simulation_structure.hpp"
#include "ecos/util/plotter.hpp"

#include <spdlog/stopwatch.h>

using namespace ecos;

inline void addConnections(simulation_structure& ss)
{
    ss.make_connection<double>("spring::for_xx", "mass::in_l_u");
    ss.make_connection<double>("spring::for_yx", "mass::in_l_w");
    ss.make_connection<double>("mass::out_l_u", "spring::dis_xx");
    ss.make_connection<double>("mass::out_l_w", "spring::dis_yx");
    ss.make_connection<double>("damper::df_0", "mass::in_f_u");
    ss.make_connection<double>("damper::df_1", "mass::in_f_w");
    ss.make_connection<double>("mass::out_f_u", "damper::lv_0");
    ss.make_connection<double>("mass::out_f_w", "damper::lv_1");
}

inline void addParameterSets(simulation_structure& ss)
{
    std::map<variable_identifier, scalar_value> map;
    map["spring::springStiffness"] = 5.;
    map["spring::zeroForceLength"] = 5.;
    map["damper::dampingCoefficient"] = 2.;
    map["mass::initialPositionX"] = 6.;
    map["mass::mediumDensity"] = 1.;
    ss.add_parameter_set("initialValues", map);
}

inline void run(simulation_structure& ss, double stopTime = 10)
{
    set_logging_level(log::level::debug);

    try {

        spdlog::stopwatch sw;

        const auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0 / 100));
        auto csvWriter = std::make_unique<csv_writer>("results/mass_spring_damper.csv");
        const auto outputPath = csvWriter->output_path();
        sim->add_listener("csv_writer", std::move(csvWriter));

        sim->init("initialValues");
        sim->step_until(stopTime);
        sim->terminate();

        log::info("Elapsed {:.4f}s", sw);

        plot_csv(outputPath, std::string(DATA_FOLDER) + "/fmus/1.0/mass_spring_damper/ChartConfig.xml");

    } catch (const std::exception& ex) {

        log::err(ex.what());
    }
}

#endif // SMD_COMMON_HPP
