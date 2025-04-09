
#include "unzipper.hpp"

#include <iomanip>
#include <string>

bool ecos::unzip(const std::filesystem::path& zip_file, const std::filesystem::path& tmp_path)
{

    if (!exists(zip_file) || !exists(tmp_path)) {
        return false;
    }

    std::ostringstream oss;
#ifdef _WIN32
    // Windows: Use the built-in tar command
    oss << "tar -xf " << std::quoted(zip_file.string()) << " -C " <<  std::quoted(tmp_path.string());
#else
    // Linux: Use the unzip command and supress output
    oss << "unzip -o " << std::quoted(zip_file.string()) << " -d " <<  std::quoted(tmp_path.string()) << " > /dev/null 2>&1";
#endif

    const auto command = oss.str();
    if (const int result = std::system(command.c_str()); result != 0) {
        return false;
    }

    return true;
}
