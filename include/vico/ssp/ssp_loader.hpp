
#ifndef VICO_SSP_LOADER_HPP
#define VICO_SSP_LOADER_HPP

#include "vico/structure/simulation_structure.hpp"

#include <filesystem>

namespace vico
{

simulation_structure load_ssp(const std::filesystem::path& path);

} // namespace vico

#endif // VICO_SSP_LOADER_HPP
