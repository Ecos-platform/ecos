#include <vico/ssp/ssp_loader.hpp>
#include <vico/simulation.hpp>
#include <vico/fixed_step_algorithm.hpp>

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
    auto ss = load_ssp("../data/ssp/quarter_truck");
    auto sim = ss.load(std::make_unique<fixed_step_algorithm>(1.0/100), "initialValues");

    auto p = sim->get_property<double>("chassis.zChassis");
    p->set_output_modifier(&dummyModifier);

    sim->init();

    while (sim->time() < 1) {
        sim->step();
        std::cout << p->operator()() << std::endl;
    }

    sim->terminate();
}
