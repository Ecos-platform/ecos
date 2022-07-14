
#include "fmi_model_sub_resolver.hpp"

#include "ecos/fmi/fmi_model.hpp"

using namespace ecos;


std::unique_ptr<model> fmi_model_sub_resolver::resolve(const std::filesystem::path& base, const std::string& uri)
{
    const auto fmuFile = base / uri;
    if (!std::filesystem::exists(fmuFile)) { return nullptr; }
    if (fmuFile.extension() != ".fmu") { return nullptr; }
    return std::make_unique<fmi_model>(fmuFile);
}
