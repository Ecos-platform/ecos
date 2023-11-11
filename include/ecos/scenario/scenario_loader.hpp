
#ifndef LIBECOS_SCENARIO_LOADER_HPP
#define LIBECOS_SCENARIO_LOADER_HPP

#include "ecos/simulation.hpp"

#include <filesystem>


namespace ecos
{

void load_scenario(simulation& sim, const std::filesystem::path& config);

}

#endif // LIBECOS_SCENARIO_LOADER_HPP
