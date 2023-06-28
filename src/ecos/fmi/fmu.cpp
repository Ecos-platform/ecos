
#include "fmi1/fmi1_fmu.hpp"
#include "fmi2/fmi2_fmu.hpp"

#include <fmilib.h>

using namespace ecos;

std::unique_ptr<fmu> loadFmu(const std::filesystem::path& fmuPath, bool fmiLogging)
{
    auto ctx = std::make_unique<fmicontext>(fmiLogging);

    const std::string fmuName = filesystem::path(fmuPath).stem().string();
    auto tmp = std::make_shared<temp_dir>(fmuName);

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
