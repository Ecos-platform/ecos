
#include "unzipper.hpp"

#include <fstream>
#include <string>
#include <vector>

bool ecos::unzip(const std::filesystem::path& zip_file, const std::filesystem::path& tmp_path)
{

    if (!exists(zip_file)) {
        return false;
    }

#ifdef _WIN32
    // Windows: Use the built-in tar command
    std::string command = "tar -xf \"" + zip_file.string() + "\" -C \"" + tmp_path.string() + "\"";
#else
    // Linux: Use the unzip command and supress output
    std::string command = "unzip -o \"" + zip_file.string() + "\" -d \"" + tmp_path.string() + "\" > /dev/null 2>&1";
#endif

    const int result = std::system(command.c_str());

    if (result != 0) {
        return false;
    }

    return true;
}
