
#include "vico/algorithm/fixed_step_algorithm.hpp"
#include "vico/listeners/csv_writer.hpp"
#include "vico/structure/simulation_structure.hpp"

#include <iostream>

using namespace vico;

int main()
{
    spdlog::set_level(spdlog::level::debug);

    simulation_structure ss;
    const std::filesystem::path fmuDir("../../data/fmus/2.0/quarter-truck");

    ss.add_model("chassis", fmuDir / "chassis.fmu");
    ss.add_model("ground", fmuDir / "ground.fmu");
    ss.add_model("wheel", fmuDir / "wheel.fmu");

    ss.make_connection<double>("chassis::p.e", "wheel::p1.e");
    ss.make_connection<double>("wheel::p1.f", "chassis::p.f");
    ss.make_connection<double>("wheel::p.e", "ground::p.e");
    ss.make_connection<double>("ground::p.f", "wheel::p.f");

    std::map<variable_identifier, std::variant<double, int, bool, std::string>> map;
    map[variable_identifier{"chassis::C.mChassis"}] = 400.0;
    ss.add_parameter_set("initialValues", map);

    auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0 / 100));
    auto p = sim->get_real_property("chassis::zChassis");

    const auto& config = csv_config::parse("../../data/ssp/quarter_truck/LogConfig.xml");
    auto csvWriter = std::make_unique<csv_writer>("results/quarter_truck_with_config.csv", config);
    csvWriter->enable_plotting("../../data/ssp/quarter_truck/ChartConfig.xml");
    sim->add_listener(std::move(csvWriter));

    sim->init("initialValues");
    sim->step_until(5);
    std::cout << p->operator()() << std::endl;

    sim->terminate();
}