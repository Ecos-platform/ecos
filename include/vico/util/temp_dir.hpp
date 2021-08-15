
#ifndef VICO_TEMP_DIR_HPP
#define VICO_TEMP_DIR_HPP

#include "fs_portability.hpp"

namespace vico
{

class temp_dir
{
private:
    const filesystem::path path_;

public:
    explicit temp_dir(const std::string& name);

    [[nodiscard]] filesystem::path path()
    {
        return path_;
    }

    ~temp_dir();
};

} // namespace vico


#endif //VICO_TEMP_DIR_HPP