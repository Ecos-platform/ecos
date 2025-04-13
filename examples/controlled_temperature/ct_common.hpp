
#ifndef LIBECOS_CT_COMMON_HPP
#define LIBECOS_CT_COMMON_HPP

#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/model_resolver.hpp"
#include "ecos/simulation.hpp"
#include "ecos/util/plotter.hpp"


using namespace ecos;

inline double kelvi2deg(double k)
{
    return k - 273.15;
}

inline int run(const std::filesystem::path& fmuPath, bool remoting)
{
    set_logging_level(log::level::debug);

    try {
        const auto resolver = default_model_resolver();
        const auto fmuModel = resolver->resolve(remoting ? ("proxyfmu://localhost?file=" + fmuPath.string()) : fmuPath.string());

        if (!fmuModel) {
            return 1;
        }

        simulation sim(std::make_unique<fixed_step_algorithm>(1.0 / 50));
        sim.add_slave(fmuModel->instantiate("slave"));

        const auto t_room = sim.get_real_property("slave::Temperature_Room");
        t_room->set_output_modifier(kelvi2deg);

        const auto t_ref = sim.get_real_property("slave::Temperature_Reference");
        t_ref->set_output_modifier(kelvi2deg);

        auto csvWriter = std::make_unique<csv_writer>("results/controlled_temperature.csv");
        const auto outputPath = csvWriter->output_path();
        sim.add_listener("csv_writer", std::move(csvWriter));

        sim.init();
        sim.step_for(10);

        sim.terminate();

        plot_csv(outputPath, fmuPath.parent_path() / "ChartConfig.xml");
    } catch (const std::exception& ex) {

        log::err(ex.what());
        return 1;
    }

    return 0;
}

#endif // LIBECOS_CT_COMMON_HPP
