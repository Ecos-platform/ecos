
#include "ecos/util/temp_dir.hpp"

#include "uuid.hpp"

#include "ecos/logger/logger.hpp"

using namespace ecos;

namespace
{

std::filesystem::path safe_temp_directory_path() {
    try {
        return std::filesystem::temp_directory_path(); // fails in debug under WSL
    } catch (const std::filesystem::filesystem_error& e) {
        log::warn("temp_directory_path() failed: {}", e.what());
        // Fallback to /tmp (standard on Unix)
        return "/tmp";
    }
}
}

temp_dir::temp_dir(const std::string& name)
    : path_(safe_temp_directory_path() /= "ecos_" + name + "_" + generate_uuid())
{
    create_directories(path_);
}

temp_dir::~temp_dir()
{
    std::error_code status;
    remove_all(path_, status);
    if (status) {
        log::warn("Failed to remove temp folder '{}': {}", path_.string(), status.message());
    }
}
