
#include "fmi1/fmi1_fmu.hpp"
#include "fmi2/fmi2_fmu.hpp"

#include "ecos/logger/logger.hpp"

#include <fmilib.h>

std::unique_ptr<fmilibcpp::fmu> fmilibcpp::loadFmu(const std::filesystem::path& fmuPath, bool fmiLogging)
{

    if (!exists(fmuPath)) {
        ecos::log::err("No such file: {}", absolute(fmuPath).string());
        return nullptr;
    }

    auto ctx = std::make_unique<fmicontext>(fmiLogging);

    const std::string fmuName = std::filesystem::path(fmuPath).stem().string();
    auto tmp = std::make_shared<ecos::temp_dir>(fmuName);

    fmi_version_enu_t version = fmi_import_get_fmi_version(ctx->ctx_, fmuPath.string().c_str(), tmp->path().string().c_str());
    if (version == fmi_version_1_enu) {
        return std::make_unique<fmi1_fmu>(std::move(ctx), tmp, fmiLogging);
    } else if (version == fmi_version_2_0_enu) {
        return std::make_unique<fmi2_fmu>(std::move(ctx), tmp, fmiLogging);
    } else {
        // TODO
    }

    return nullptr;
}
