
#ifndef LIBECOS_SCENARIO_LOADER_HPP
#define LIBECOS_SCENARIO_LOADER_HPP

#include <filesystem>
#include "ecos/simulation.hpp"


namespace ecos {

void load_scenario(simulation& sim, const std::filesystem::path& config);

}

#endif // LIBECOS_SCENARIO_LOADER_HPP
