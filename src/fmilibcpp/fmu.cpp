
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

    // auto ctx = std::make_unique<fmicontext>(fmiLogging);

    const std::string fmuName = std::filesystem::path(fmuPath).stem().string();
    // auto tmp = std::make_shared<ecos::temp_dir>(fmuName);

    auto handle = fmi4c_loadFmu(fmuPath.string().c_str(), "dummy");

    auto version = fmi1_getVersion(handle);
    if (version[0] == '1') {
        return std::make_unique<fmi1_fmu>(handle, fmiLogging);
    } else if (version[0] == '2') {
        return std::make_unique<fmi2_fmu>(handle, fmiLogging);
    } else {
        // TODO
    }

    return nullptr;
}
