
#include "ecos/simulation.hpp"
#include "ecos/structure/simulation_structure.hpp"
#include <ecos/algorithm/fixed_step_algorithm.hpp>
#include <ecos/listeners/csv_writer.hpp>
#include <ecos/logger/logger.hpp>
#include <ecos/util/plotter.hpp>

#include <iostream>

using namespace ecos;

int main()
{

    set_logging_level(log::level::debug);

    std::string resultFile{"results/bouncing_ball.csv"};
    const std::string fmuPath{std::string(DATA_FOLDER) + "/fmus/3.0/ref/BouncingBall.fmu"};

    try {
        simulation_structure ss;
        ss.add_model("bouncing_ball", "proxyfmu:://localhost?file=" + fmuPath);

        const auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0 / 60));
        sim->add_listener("csv_writer", std::make_unique<csv_writer>(resultFile));

        sim->init();
        sim->step_for(10);
        sim->terminate();

        TChartConfig config;
        config.addChart(TTimeSeriesChart{
            "bouncing_ball",
            "Time[s]",
            {{{"bouncing_ball", {{
                                    "h",
                                }}}}}});

        plot_csv(resultFile, config);

    } catch (const std::runtime_error& e) {
        std::cerr << "Exception raised: " << e.what() << std::endl;
    }
}
