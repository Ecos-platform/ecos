
#include "ecos/util/temp_dir.hpp"

#include "uuid.hpp"

#include "ecos/logger/logger.hpp"

using namespace ecos;

temp_dir::temp_dir(const std::string& name)
    : path_(std::filesystem::temp_directory_path() /= "ecos_" + name + "_" + generate_uuid())
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
