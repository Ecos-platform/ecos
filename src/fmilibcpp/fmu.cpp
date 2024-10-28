
#include "fmi1/fmi1_fmu.hpp"
#include "fmi2/fmi2_fmu.hpp"

#include "ecos/logger/logger.hpp"

#include <fmi4c.h>

std::unique_ptr<fmilibcpp::fmu> fmilibcpp::loadFmu(const std::filesystem::path& fmuPath, bool fmiLogging)
{

    if (!exists(fmuPath)) {
        ecos::log::err("No such file: {}", absolute(fmuPath).string());
        return nullptr;
    }

    const std::string fmuName = std::filesystem::path(fmuPath).stem().string();
    auto handle = std::make_unique<fmicontext>(fmi4c_loadFmu(fmuPath.string().c_str(), "dummy"), fmiLogging);

    const auto version = fmi4c_getFmiVersion(handle->ctx_);
    if (version == fmiVersion1) {
        return std::make_unique<fmi1_fmu>(std::move(handle), fmiLogging);
    } else if (version == fmiVersion2) {
        return std::make_unique<fmi2_fmu>(std::move(handle), fmiLogging);
    } else {
        // TODO
    }

    return nullptr;
}
