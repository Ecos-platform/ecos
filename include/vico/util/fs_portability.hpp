
#ifndef VICO_FS_PORTABILITY_HPP
#define VICO_FS_PORTABILITY_HPP

#if __has_include(<filesystem>)
#    include <filesystem>
namespace vico
{
namespace fs = std::filesystem;
}
#else
#    include <experimental/filesystem>
namespace vico
{
namespace fs = std::experimental::filesystem;
}
#endif

#endif // VICO_FS_PORTABILITY_HPP
