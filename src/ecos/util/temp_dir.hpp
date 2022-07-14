
#ifndef ECOS_TEMP_DIR_HPP
#define ECOS_TEMP_DIR_HPP

#include <filesystem>

namespace ecos
{

class temp_dir
{
private:
    const std::filesystem::path path_;

public:
    explicit temp_dir(const std::string& name);

    temp_dir(const temp_dir&) = delete;

    [[nodiscard]] std::filesystem::path path()
    {
        return path_;
    }

    ~temp_dir();
};

} // namespace ecos


#endif // ECOS_TEMP_DIR_HPP
