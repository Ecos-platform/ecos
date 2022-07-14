
#include "proxy_model_sub_resolver.hpp"

#include "ecos/fmi/proxy/proxy_model.hpp"

using namespace ecos;

std::unique_ptr<model> proxy_model_sub_resolver::resolve(const std::filesystem::path& base, const std::string& uri)
{
    if (uri.rfind("proxyfmu", 0) == 0) {
        const auto find = uri.find("file=", 0);
        if (find == std::string::npos) {
            throw std::runtime_error("proxyfmu source missing file= component..");
        }
        const auto fmuFile = base / uri.substr(find + 5);
        return std::make_unique<proxy_model>(fmuFile);
    }
    return nullptr;
}
