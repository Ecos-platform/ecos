
#ifndef ECOS_UNZIPPER_HPP
#define ECOS_UNZIPPER_HPP

#include <filesystem>

namespace ecos
{

bool unzip(const std::filesystem::path& zip_file, const std::filesystem::path& tmp_path);

} // namespace ecos

#endif // ECOS_UNZIPPER_HPP
