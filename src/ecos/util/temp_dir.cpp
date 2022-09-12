
#include "ecos/util/temp_dir.hpp"

#include "uuid.hpp"

#include "ecos/logger.hpp"

using namespace ecos;

temp_dir::temp_dir(const std::string& name)
    : path_(std::filesystem::temp_directory_path() /= "ecos_" + name + "_" + generate_uuid())
{
    std::filesystem::create_directories(path_);
}

temp_dir::~temp_dir()
{
    std::error_code status;
    std::filesystem::remove_all(path_, status);
    if (status) {
        warn("Failed to remove temp folder '{}': {}", path_.string(), status.message());
    }
}
