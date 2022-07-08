
#include "vico/listeners/simulation_listener.hpp"

#include "vico/simulation.hpp"

namespace vico
{

void simulation_listener::pre_init(simulation& sim) { }
void simulation_listener::post_init(simulation& sim) { }

void simulation_listener::pre_step(simulation& sim) { }
void simulation_listener::post_step(simulation& sim) { }

void simulation_listener::post_terminate(simulation& sim) { }

} // namespace vico
