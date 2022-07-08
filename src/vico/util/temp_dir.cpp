
#include "vico/util/temp_dir.hpp"

#include "spdlog/spdlog.h"
#include "uuid.hpp"

#include <string>

using namespace vico;

temp_dir::temp_dir(const std::string& name)
    : path_(std::filesystem::temp_directory_path() /= "vico_" + name + "_" + generate_uuid())
{
    std::filesystem::create_directories(path_);
}

temp_dir::~temp_dir()
{
    std::error_code status;
    std::filesystem::remove_all(path_, status);
    if (status) {
        spdlog::warn("Failed to remove temp folder '{}': ", path_.string(), status.message());
    }
}
