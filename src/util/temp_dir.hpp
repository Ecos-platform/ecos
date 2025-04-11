
#ifndef ECOS_TEMP_DIR_HPP
#define ECOS_TEMP_DIR_HPP

#include "util/uuid.hpp"

#include "ecos/logger/logger.hpp"

#include <filesystem>

namespace ecos
{

class temp_dir
{

public:
    explicit temp_dir(const std::string& name)
        : path_(safe_temp_directory_path() /= "ecos_" + name + "_" + generate_uuid())
    {
        create_directories(path_);
    }

    temp_dir(temp_dir&&) = delete;
    temp_dir(const temp_dir&) = delete;
    temp_dir& operator=(temp_dir&&) = delete;
    temp_dir& operator=(const temp_dir&) = delete;


    [[nodiscard]] std::filesystem::path path() const noexcept
    {
        return path_;
    }

    ~temp_dir()
    {
        std::error_code status;
        remove_all(path_, status);
        if (status) {
            log::warn("Failed to remove temp folder '{}': {}", path_.string(), status.message());
        }
    }

private:
    const std::filesystem::path path_;

    static std::filesystem::path safe_temp_directory_path()
    {
        try {
            return std::filesystem::temp_directory_path(); // fails in debug under WSL
        } catch (const std::filesystem::filesystem_error& e) {
            log::warn("temp_directory_path() failed: {}", e.what());
            // Fallback to /tmp (standard on Unix)
            return "/tmp";
        }
    }
};

} // namespace ecos


#endif // ECOS_TEMP_DIR_HPP
