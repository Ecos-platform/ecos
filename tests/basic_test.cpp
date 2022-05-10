#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vico/fmi/fmi_system.hpp>
#include <vico/simulation.hpp>

#include <fmilibcpp/fmu.hpp>

using namespace vico;
using namespace fmilibcpp;

TEST_CASE("basic_test")
{
    std::string fmuPath("../fmus/2.0/20sim/ControlledTemperature.fmu");
    auto fmu = loadFmu(fmuPath);

    simulation sim(1.0 / 100);

    auto algorithm = std::make_unique<fixed_step_algorithm>();
    auto sys = std::make_unique<fmi_system>(std::move(algorithm));
    sys->add_slave(fmu->new_instance("slave"));

    sim.add_system(std::move(sys));

    auto p = sim.get_property<double>("slave.Temperature_Room");
    p->add_modifier([](double value) {
        return value * 10;
    });

    sim.init();
    std::cout << p->get_value() << std::endl;

    sim.step();

    std::cout << p->get_value() << std::endl;


    sim.terminate();
}