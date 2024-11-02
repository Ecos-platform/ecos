
#ifndef LIBECOS_LOGGER_HPP
#define LIBECOS_LOGGER_HPP

#include <spdlog/fmt/fmt.h>

namespace ecos::log
{

enum class level : int
{
    trace,
    debug,
    info,
    warn,
    err,
    off
};

void set_logging_level(level lvl);

void log(level lvl, std::string_view msg);

template<typename... Args>
void trace(fmt::format_string<Args...> fmt, Args&&... args)
{
    log(level::trace, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void debug(fmt::format_string<Args...> fmt, Args&&... args)
{
    log(level::debug, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void info(fmt::format_string<Args...> fmt, Args&&... args)
{
    log(level::info, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void warn(fmt::format_string<Args...> fmt, Args&&... args)
{
    log(level::warn, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void err(fmt::format_string<Args...> fmt, Args&&... args)
{
    log(level::err, fmt::format(fmt, std::forward<Args>(args)...));
}

inline void trace(std::string_view msg)
{
    log(level::trace, msg);
}

inline void debug(std::string_view msg)
{
    log(level::debug, msg);
}

inline void info(std::string_view msg)
{
    log(level::info, msg);
}

inline void warn(std::string_view msg)
{
    log(level::warn, msg);
}

inline void err(std::string_view msg)
{
    log(level::err, msg);
}

} // namespace ecos::log


#endif // LIBECOS_LOGGER_HPP
