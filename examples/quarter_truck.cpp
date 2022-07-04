#include <vico/fmi/fixed_step_algorithm.hpp>
#include <vico/fmi/fmi_model.hpp>
#include <vico/fmi/fmi_model_instance.hpp>
#include <vico/simulation.hpp>

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
    simulation sim(std::make_unique<fixed_step_algorithm>(1.0 / 100));
    auto chassisModel = fmi_model("../data/fmus/2.0/quarter-truck/chassis.fmu");
    auto groundModel = fmi_model("../data/fmus/2.0/quarter-truck/ground.fmu");
    auto wheelModel = fmi_model("../data/fmus/2.0/quarter-truck/wheel.fmu");

    sim.add_slave(chassisModel.instantiate("chassis"));
    sim.add_slave(groundModel.instantiate("ground"));
    sim.add_slave(wheelModel.instantiate("wheel"));

    sim.add_connection<double>("chassis.p.e", "wheel.p1.e");
    sim.add_connection<double>("wheel.p1.f", "chassis.p.f");
    sim.add_connection<double>("wheel.p.e", "ground.p.e");
    sim.add_connection<double>("ground.p.f", "wheel.p.f");

    auto p = sim.get_property<double>("chassis.zChassis");
    p->set_output_modifier(&dummyModifier);

    sim.init();

    while (sim.time() < 10) {
        sim.step();
        std::cout << p->operator()() << std::endl;
    }

    sim.terminate();
}
