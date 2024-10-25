
#include "ecos/logger/logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace ecos;

namespace
{

spdlog::level::level_enum convert(log::level lvl)
{
    if (lvl == log::level::trace) {
        return spdlog::level::trace;
    } else if (lvl == log::level::debug) {
        return spdlog::level::debug;
    } else if (lvl == log::level::info) {
        return spdlog::level::info;
    } else if (lvl == log::level::warn) {
        return spdlog::level::warn;
    } else if (lvl == log::level::err) {
        return spdlog::level::err;
    } else if (lvl == log::level::off) {
        return spdlog::level::off;
    } else {
        throw std::runtime_error("Invalid log level");
    }
}

} // namespace

struct ecos_logger
{

    ecos_logger(const ecos_logger&) = delete;
    void operator=(const ecos_logger&) = delete;

    void set_level(log::level lvl)
    {
        logger_->set_level(convert(lvl));
    }

    void log(log::level lvl, std::string_view msg)
    {
        logger_->log(convert(lvl), msg);
    }

    static ecos_logger& get_instance()
    {
        static ecos_logger l;
        return l;
    }

private:
    std::shared_ptr<spdlog::logger> logger_;

    ecos_logger()
        : logger_(spdlog::stdout_color_mt("ecos"))
    { }
};

void log::set_logging_level(level lvl)
{
    ecos_logger::get_instance().set_level(lvl);
}

void log::log(level lvl, std::string_view msg)
{
    ecos_logger::get_instance().log(lvl, msg);
}
