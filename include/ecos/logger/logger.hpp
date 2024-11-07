
#ifndef LIBECOS_LOGGER_HPP
#define LIBECOS_LOGGER_HPP

#include <spdlog/spdlog.h>

namespace ecos::log
{

void create_default_logger(spdlog::level::level_enum lvl = spdlog::level::warn);

} // namespace ecos::log


#endif // LIBECOS_LOGGER_HPP
