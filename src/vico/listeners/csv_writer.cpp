
#include "vico/listeners/csv_writer.hpp"

#include "vico/simulation.hpp"


using namespace vico;

void csv_writer::post_init(simulation& sim)
{
    for (auto& instance : sim.get_instances()) {
        auto& properties = instance->get_properties();
    }
}

void csv_writer::post_step(simulation& sim)
{
}

void csv_writer::post_terminate(simulation& sim)
{
}
