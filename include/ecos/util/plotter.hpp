
#ifndef ECOS_PLOTTER_HPP
#define ECOS_PLOTTER_HPP

#include <filesystem>

namespace ecos
{

void plot_csv(const std::filesystem::path& csvFile, const std::filesystem::path& plotConfig);

} // namespace ecos

#endif // ECOS_PLOTTER_HPP
