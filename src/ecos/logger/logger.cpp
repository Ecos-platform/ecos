
#include "ecos/logger/logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace ecos;


void log::create_default_logger(spdlog::level::level_enum lvl)
{
    auto logger = spdlog::stdout_color_mt("ecos");
    logger->set_level(lvl);
    set_default_logger(std::move(logger));
}
