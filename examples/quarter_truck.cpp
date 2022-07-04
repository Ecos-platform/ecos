#include <vico/fixed_step_algorithm.hpp>
#include <vico/fmi/fmi_model.hpp>
#include <vico/fmi/fmi_model_instance.hpp>
#include <vico/simulation.hpp>
#include <vico/structure/simulation_structure.hpp>

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

    simulation_structure ss;

    ss.add_model("chassis", std::make_shared<fmi_model>("../data/fmus/2.0/quarter-truck/chassis.fmu"));
    ss.add_model("ground", std::make_shared<fmi_model>("../data/fmus/2.0/quarter-truck/ground.fmu"));
    ss.add_model("wheel", std::make_shared<fmi_model>("../data/fmus/2.0/quarter-truck/wheel.fmu"));

    ss.make_connection<double>("chassis.p.e", "wheel.p1.e");
    ss.make_connection<double>("wheel.p1.f", "chassis.p.f");
    ss.make_connection<double>("wheel.p.e", "ground.p.e");
    ss.make_connection<double>("ground.p.f", "wheel.p.f");

    std::map<variable_identifier, std::variant<double, int, bool, std::string>> map;
    map[variable_identifier{"chassis.C.mChassis"}] = 4000.0;
    ss.add_parameter_set("initialValues", map);

    auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0 / 100), "initialValues");

    auto p = sim->get_property<double>("chassis.zChassis");
    p->set_output_modifier(&dummyModifier);

    sim->init();

    while (sim->time() < 1) {
        sim->step();
        std::cout << p->operator()() << std::endl;
    }

    sim->terminate();
}
