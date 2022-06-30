
#ifndef VICO_SSP_LOADER_HPP
#define VICO_SSP_LOADER_HPP

#include "vico/structure/simulation_structure.hpp"
#include "vico/util/fs_portability.hpp"

namespace vico
{

simulation_structure load_ssp(const fs::path& path);

} // namespace vico

#endif // VICO_SSP_LOADER_HPP
