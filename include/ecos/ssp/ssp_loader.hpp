
#ifndef ECOS_SSP_LOADER_HPP
#define ECOS_SSP_LOADER_HPP

#include "ecos/structure/simulation_structure.hpp"

#include <filesystem>
#include <memory>

namespace ecos
{

/**
 * \brief Load a simulation structure from a SSP file.
 *
 * This function reads an SSP file or folder and constructs a simulation_structure object.
 *
 * \param path The path to the SSP file/folder.
 * \return A unique pointer to the loaded simulation_structure.
 */
[[nodiscard]] std::unique_ptr<simulation_structure> load_ssp(const std::filesystem::path& path);

} // namespace ecos

#endif // ECOS_SSP_LOADER_HPP
