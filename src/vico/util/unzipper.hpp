
#ifndef FMI4CPP_UNZIPPER_HPP
#define FMI4CPP_UNZIPPER_HPP

#include <filesystem>

namespace vico
{

bool unzip(const std::filesystem::path& zip_file, const std::filesystem::path& tmp_path);

} // namespace vico

#endif // FMI4CPP_UNZIPPER_HPP
