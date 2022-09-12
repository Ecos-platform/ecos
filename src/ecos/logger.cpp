
#include "ecos/logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace ecos;

namespace
{

spdlog::level::level_enum convert(ecos_logger::level lvl)
{
    if (lvl == ecos_logger::level::trace) {
        return spdlog::level::trace;
    } else if (lvl == ecos_logger::level::debug) {
        return spdlog::level::debug;
    } else if (lvl == ecos_logger::level::info) {
        return spdlog::level::info;
    } else if (lvl == ecos_logger::level::warn) {
        return spdlog::level::warn;
    } else if (lvl == ecos_logger::level::err) {
        return spdlog::level::err;
    } else if (lvl == ecos_logger::level::off) {
        return spdlog::level::off;
    } else {
        throw std::runtime_error("Invalid log level");
    }
}

} // namespace

struct ecos_logger::impl
{

    impl()
        : logger_(spdlog::stdout_color_mt("ecos"))
    { }

    ~impl() = default;

    void set_level(level lvl)
    {
        if (lvl == ecos_logger::level::trace) {
            logger_->set_level(spdlog::level::trace);
        } else if (lvl == ecos_logger::level::debug) {
            logger_->set_level(spdlog::level::debug);
        } else if (lvl == ecos_logger::level::info) {
            logger_->set_level(spdlog::level::info);
        } else if (lvl == ecos_logger::level::warn) {
            logger_->set_level(spdlog::level::warn);
        } else if (lvl == ecos_logger::level::err) {
            logger_->set_level(spdlog::level::err);
        } else if (lvl == ecos_logger::level::off) {
            logger_->set_level(spdlog::level::off);
        } else {
            throw std::runtime_error("Invalid log level");
        }
    }

    void log(ecos_logger::level lvl, fmt::basic_string_view<char> fmt)
    {
        logger_->log(convert(lvl), fmt);
    }


private:
    std::shared_ptr<spdlog::logger> logger_;
};

void ecos_logger::set_level(level lvl)
{
    pimpl_->set_level(lvl);
}

void ecos_logger::log(ecos_logger::level lvl, const std::string& msg)
{
    pimpl_->log(lvl, msg);
}

ecos::ecos_logger::ecos_logger(): pimpl_(std::make_unique<impl>()) {}
ecos::ecos_logger::~ecos_logger() = default;
