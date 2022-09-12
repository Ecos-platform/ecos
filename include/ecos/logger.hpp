
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

    template<typename... Args>
    void trace(fmt::format_string<Args...> fmt, Args &&... args) {
        log(level::trace, fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void debug(fmt::format_string<Args...> fmt, Args &&... args) {
        log(level::debug, fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void info(fmt::format_string<Args...> fmt, Args &&... args) {
        log(level::info, fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void warn(fmt::format_string<Args...> fmt, Args &&... args) {
        log(level::warn, fmt::format(fmt, std::forward<Args>(args)...));
    }

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

inline ecos_logger& logger() {
    return ecos_logger::get_instance();
}

} // namespace ecos



#endif // LIBECOS_LOGGER_HPP
