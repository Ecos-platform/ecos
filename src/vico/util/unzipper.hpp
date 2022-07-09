
#ifndef VICO_UNZIPPER_HPP
#define VICO_UNZIPPER_HPP

#include <filesystem>

namespace vico
{

bool unzip(const std::filesystem::path& zip_file, const std::filesystem::path& tmp_path);

} // namespace vico

#endif // VICO_UNZIPPER_HPP
