
#ifndef VICO_TEMP_DIR_HPP
#define VICO_TEMP_DIR_HPP

#include "vico/util/fs_portability.hpp"

namespace vico
{

class temp_dir
{
private:
    const fs::path path_;

public:
    explicit temp_dir(const std::string& name);

    temp_dir(const temp_dir&) = delete;

    [[nodiscard]] fs::path path()
    {
        return path_;
    }

    ~temp_dir();
};

} // namespace vico


#endif // VICO_TEMP_DIR_HPP