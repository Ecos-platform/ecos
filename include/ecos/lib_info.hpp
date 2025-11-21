
#ifndef LIBECOS_LIB_INFO_HPP
#define LIBECOS_LIB_INFO_HPP

#include <ostream>

namespace ecos
{

/// Software version
struct version
{
    int major = 0;
    int minor = 0;
    int patch = 0;
};

/// Returns the version of the ecos library.
version library_version();

inline std::ostream& operator<<(std::ostream& os, const version& v)
{
    os << v.major << "." << v.minor << "." << v.patch;
    return os;
}

} // namespace ecos

#endif // LIBECOS_LIB_INFO_HPP
