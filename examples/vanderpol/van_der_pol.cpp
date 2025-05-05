
#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/simulation.hpp"
#include "ecos/structure/simulation_structure.hpp"
#include "ecos/util/plotter.hpp"

using namespace ecos;

int main()
{

    set_logging_level(log::level::debug);

    std::string resultFile{"results/vanderpol.csv"};
    const std::string fmuPath{std::string(DATA_FOLDER) + "/fmus/3.0/ref/VanDerPol.fmu"};

    simulation_structure ss;
    ss.add_model("model", fmuPath);

    const auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0 / 100));
    sim->add_listener("csv_writer", std::make_unique<csv_writer>(resultFile));

    sim->init();

    double mu = 0.01;
    while (mu < 5) {
        sim->get_real_property("model::mu")->set_value(mu);
        sim->step(1000);
        mu += mu / 5;
    }
    sim->terminate();

    TChartConfig config;
    config.addChart(TXYSeriesChart{
        "Van der Pol Oscillator",
        "Position (x)",
        "Velocity (y)",
        {TXYSeries{"test", "model::x0", "model::der(x0)"}}});

    plot_csv(resultFile, config);
}
