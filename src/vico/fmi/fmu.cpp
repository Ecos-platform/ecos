
#include "vico/fmi/fmi1/fmi1_fmu.hpp"
#include "vico/fmi/fmi2/fmi2_fmu.hpp"
#include "vico/util/temp_dir.hpp"

#include <fmilib.h>

namespace vico
{

std::unique_ptr<fmu> loadFmu(const filesystem::path& fmuPath)
{
    auto ctx = std::make_unique<fmicontext>();

    const std::string fmuName = filesystem::path(fmuPath).stem().string();
    auto tmp = std::make_shared<temp_dir>(fmuName);

    fmi_version_enu_t version = fmi_import_get_fmi_version(ctx->ctx_, fmuPath.string().c_str(), tmp->path().string().c_str());
    if (version == fmi_version_1_enu) {
        return std::make_unique<fmi1_fmu>(std::move(ctx), tmp);
    } else if (version == fmi_version_2_0_enu) {
        return std::make_unique<fmi2_fmu>(std::move(ctx), tmp);
    } else {
        // TODO
    }

    return nullptr;
}

} // namespace vico