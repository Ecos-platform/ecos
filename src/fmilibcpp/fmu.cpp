
#include "fmi1/fmi1_fmu.hpp"
#include "fmi2/fmi2_fmu.hpp"
#include "fmi3/fmi3_fmu.hpp"

#include "ecos/logger/logger.hpp"
#include "ecos/util/temp_dir.hpp"
#include "ecos/util/unzipper.hpp"

#include <fmi4c.h>

std::unique_ptr<fmilibcpp::fmu> fmilibcpp::loadFmu(const std::filesystem::path& fmuPath, bool fmiLogging)
{

    if (!exists(fmuPath)) {
        ecos::log::err("No such file: {}", absolute(fmuPath).string());
        return nullptr;
    }

    const std::string fmuName = std::filesystem::path(fmuPath).stem().string();
    auto temp = std::make_unique<ecos::temp_dir>(fmuName);

    if (!ecos::unzip(fmuPath, temp->path())) {
        ecos::log::err("Failed to unzip '{}' to tempdir '{}'!", fmuPath.string(), temp->path().string());
        return nullptr;
    }

    auto fmuCtx = fmi4c_loadUnzippedFmu(fmuPath.string().c_str(), temp->path().string().c_str());
    if (!fmuCtx) {
        ecos::log::err("Failed to load '{}'!", fmuPath.string());
        return nullptr;
    }

    auto handle = std::make_unique<fmicontext>(fmuCtx, std::move(temp));

    switch (fmi4c_getFmiVersion(handle->handle_)) {
        case fmiVersion1:
            return std::make_unique<fmi1_fmu>(std::move(handle), fmiLogging);
        case fmiVersion2:
            return std::make_unique<fmi2_fmu>(std::move(handle), fmiLogging);
        case fmiVersion3:
            return std::make_unique<fmi3_fmu>(std::move(handle), fmiLogging);
        default:
            return nullptr;
    }
}
