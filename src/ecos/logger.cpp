
#include "ecos/logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace
{

struct ecos_logger
{
    std::shared_ptr<spdlog::logger> logger_;

    ecos_logger()
        : logger_(spdlog::stdout_color_mt("ecos"))
    { }

    static ecos_logger get_instance()
    {
        static ecos_logger instance;
        return instance;
    }
};


} // namespace

spdlog::logger& logger()
{
    return *ecos_logger::get_instance().logger_;
}
