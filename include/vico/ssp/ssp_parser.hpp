
#ifndef VICO_SSP_PARSER_HPP
#define VICO_SSP_PARSER_HPP

#include "vico/structure/simulation_structure.hpp"
#include "vico/util/fs_portability.hpp"

namespace vico
{

simulation_structure parse_ssp(const fs::path& path);

} // namespace vico

#endif // VICO_SSP_PARSER_HPP
