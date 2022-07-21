
#ifndef LIBECOS_SCALAR_HPP
#define LIBECOS_SCALAR_HPP

#include <string>
#include <variant>

namespace ecos
{

using scalar_value = std::variant<double, int, bool, std::string>;

}


#endif // LIBECOS_SCALAR_HPP
