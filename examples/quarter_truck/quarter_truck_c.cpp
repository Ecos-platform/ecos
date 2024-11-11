
#include "ecos/ecos.h"

#include <filesystem>

// just to check if it works
double modifier(double val)
{
    return val;
}

int main()
{
    ecos_set_log_level("debug");

    const std::filesystem::path fmuDir = std::string(DATA_FOLDER) + "/ssp/quarter_truck/resources";

    const auto ss = ecos_simulation_structure_create();
    ecos_simulation_structure_add_model(ss, "chassis", (fmuDir / "chassis.fmu").string().c_str());
    ecos_simulation_structure_add_model(ss, "ground", (fmuDir / "ground.fmu").string().c_str());
    ecos_simulation_structure_add_model(ss, "wheel", (fmuDir / "wheel.fmu").string().c_str());

    ecos_simulation_structure_make_real_connection_mod(ss, "chassis::p.e", "wheel::p1.e", modifier);
    ecos_simulation_structure_make_real_connection(ss, "wheel::p1.f", "chassis::p.f");
    ecos_simulation_structure_make_real_connection(ss, "wheel::p.e", "ground::p.e");
    ecos_simulation_structure_make_real_connection(ss, "ground::p.f", "wheel::p.f");

    const auto pps = ecos_parameter_set_create();
    ecos_parameter_set_add_real(pps, "chassis::C.mChassis", 400.);
    ecos_simulation_structure_add_parameter_set(ss, "initialValues", pps);

    const auto sim = ecos_simulation_create_from_structure(ss, 1.0 / 100);

    ecos_simulation_structure_destroy(ss);
    ecos_parameter_set_destroy(pps);

    const auto logConfig = std::string(DATA_FOLDER) + "/ssp/quarter_truck/LogConfig.xml";
    const auto plotConfig = std::string(DATA_FOLDER) + "/ssp/quarter_truck/ChartConfig.xml";
    const auto resultFile = std::string{"results/quarter_truck_c_with_config.csv"};
    const auto csvWriter = ecos_csv_writer_create(resultFile.c_str(), logConfig.c_str(), plotConfig.c_str());

    ecos_simulation_add_listener(sim, "CSV Writer", csvWriter);

    ecos_simulation_init(sim, 0, "initialValues");
    ecos_simulation_step_until(sim, 10);
    ecos_simulation_terminate(sim);

    ecos_simulation_destroy(sim);
}
