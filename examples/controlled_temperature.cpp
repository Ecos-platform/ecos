#include <vico/fmi/fixed_step_algorithm.hpp>
#include <vico/simulation.hpp>

#include <fmilibcpp/fmu.hpp>

using namespace vico;

int main()
{
    std::string fmuPath("../data/fmus/2.0/20sim/ControlledTemperature.fmu");
    auto fmu = fmilibcpp::loadFmu(fmuPath);

    simulation sim(std::make_unique<fixed_step_algorithm>(1.0/100));
    sim.add_slave(fmu->new_instance("slave"));

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
