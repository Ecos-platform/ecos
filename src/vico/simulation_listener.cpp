
#include "vico/simulation_listener.hpp"

#include "vico/util/uuid.hpp"

using namespace vico;

simulation_listener::simulation_listener()
    : uuid(generate_uuid())
{ }