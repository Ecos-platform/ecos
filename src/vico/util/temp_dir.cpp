
#include "vico/util/temp_dir.hpp"

#include "random.hpp"

#include <iostream>
#include <string>

using namespace vico;

namespace
{

std::string generate_simple_id()
{
    const int len = 6;
    std::string id;
    vico::fixed_range_random_generator rng(0, 9);
    for (auto i = 0; i < len; i++) {
        id += std::to_string(rng.next());
    }
    return id;
}

} // namespace


temp_dir::temp_dir(const std::string& name)
    : path_(fs::temp_directory_path() /= "vico_" + name + "_" + generate_simple_id())
{
    fs::create_directories(path_);
}

temp_dir::~temp_dir()
{
    std::error_code status;
    fs::remove_all(path_, status);
    if (status) {
        std::cerr << "Failed to remove temp folder '" << path_.string() << "': " << status.message() << std::endl;
    }
}
