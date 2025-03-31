
#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/structure/simulation_structure.hpp"
#include "ecos/util/plotter.hpp"

using namespace ecos;

int main()
{
    set_logging_level(log::level::debug);

    const std::filesystem::path sspDir = std::string(DATA_FOLDER) + "/ssp/quarter_truck";
    const std::filesystem::path fmuDir = sspDir / "resources";

    try {
        simulation_structure ss;
        ss.add_model("chassis", fmuDir / "chassis.fmu");
        ss.add_model("ground", fmuDir / "ground.fmu");
        ss.add_model("wheel", fmuDir / "wheel.fmu");

        ss.make_connection<double>("chassis::p.e", "wheel::p1.e");
        ss.make_connection<double>("wheel::p1.f", "chassis::p.f");
        ss.make_connection<double>("wheel::p.e", "ground::p.e");
        ss.make_connection<double>("ground::p.f", "wheel::p.f");

        std::map<variable_identifier, scalar_value> map;
        map["chassis::C.mChassis"] = 400.0;
        ss.add_parameter_set("initialValues", map);

        const auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0 / 100));

        auto csvWriter = std::make_unique<csv_writer>("results/quarter_truck_with_config.csv");
        const auto outputPath = csvWriter->output_path();
        csv_config& config = csvWriter->config();
        config.load(sspDir / "CsvConfig.xml");
        sim->add_listener("csv_writer", std::move(csvWriter));

        sim->init("initialValues");
        sim->step_until(5);

        sim->terminate();

        plot_csv(outputPath, sspDir / "ChartConfig.xml");
    } catch (const std::exception& ex) {

        log::err(ex.what());
    }
}
