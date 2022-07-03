#include <vico/fmi/fixed_step_algorithm.hpp>
#include <vico/simulation.hpp>

#include <fmilibcpp/fmu.hpp>

using namespace vico;

namespace
{

double dummyModifier(double value)
{
    return value * 2;
}

} // namespace

int main()
{
    simulation sim(std::make_unique<fixed_step_algorithm>(1.0/100));

    sim.add_slave(fmilibcpp::loadFmu("../data/fmus/2.0/quarter-truck/chassis.fmu")->new_instance("chassis"));
    sim.add_slave(fmilibcpp::loadFmu("../data/fmus/2.0/quarter-truck/ground.fmu")->new_instance("ground"));
    sim.add_slave(fmilibcpp::loadFmu("../data/fmus/2.0/quarter-truck/wheel.fmu")->new_instance("wheel"));

    auto& c = sim.add_connection<double>("chassis.p.e", "wheel.p1.e");
    sim.add_connection<double>("wheel.p1.f", "chassis.p.f");
    sim.add_connection<double>("wheel.p.e", "ground.p.e");
    sim.add_connection<double>("ground.p.f", "wheel.p.f");

    auto p = sim.get_property<double>("chassis.zChassis");
    p->add_modifier(&dummyModifier);

    sim.init();

    while (sim.time() < 10) {
        sim.step();
        std::cout << p->operator()() << std::endl;
    }

    sim.terminate();
}
