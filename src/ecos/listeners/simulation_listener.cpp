
#include "ecos/listeners/simulation_listener.hpp"

#include "ecos/simulation.hpp"

namespace ecos
{

void simulation_listener::pre_init(simulation& sim) { }
void simulation_listener::post_init(simulation& sim) { }

void simulation_listener::pre_step(simulation& sim) { }
void simulation_listener::post_step(simulation& sim) { }

void simulation_listener::post_terminate(simulation& sim) { }

} // namespace ecos
