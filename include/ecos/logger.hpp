
#ifndef LIBECOS_LOGGER_HPP
#define LIBECOS_LOGGER_HPP

#include <fmt/core.h>
#include <memory>
#include <unordered_map>

namespace ecos
{

class ecos_logger
{

public:
    ~ecos_logger();

    enum class level : int
    {
        trace,
        debug,
        info,
        warn,
        err,
        off
    };

    void set_level(level lvl);

    void log(level lvl, const std::string& msg);

    static ecos_logger& get_instance()
    {
        static ecos_logger logger;
        return logger;
    }

private:
    ecos_logger();

    struct impl;
    std::unique_ptr<impl> pimpl_;
};

inline void set_logging_level(ecos_logger::level lvl)
{
    ecos_logger::get_instance().set_level(lvl);
}

template<typename... Args>
void trace(fmt::format_string<Args...> fmt, Args&&... args)
{
    ecos_logger::get_instance().log(ecos_logger::level::trace, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void debug(fmt::format_string<Args...> fmt, Args&&... args)
{
    ecos_logger::get_instance().log(ecos_logger::level::debug, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void info(fmt::format_string<Args...> fmt, Args&&... args)
{
    ecos_logger::get_instance().log(ecos_logger::level::info, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
void warn(fmt::format_string<Args...> fmt, Args&&... args)
{
    ecos_logger::get_instance().log(ecos_logger::level::warn, fmt::format(fmt, std::forward<Args>(args)...));
}

} // namespace ecos


#endif // LIBECOS_LOGGER_HPP
