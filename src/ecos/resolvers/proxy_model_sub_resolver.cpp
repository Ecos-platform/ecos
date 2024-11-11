
#include "proxy_model_sub_resolver.hpp"

#include "ecos/fmi/proxy/proxy_model.hpp"

using namespace ecos;

namespace
{
std::optional<proxy::remote_info> parse(const std::string& url)
{
    // Find the start of the host (after "proxyfmu://")
    std::size_t start_pos = url.find("://");
    if (start_pos == std::string::npos) {
        return std::nullopt; // Return empty if no protocol found
    }
    start_pos += 3; // Move past "://"

    std::size_t end_pos = url.find('?', start_pos);
    if (end_pos == std::string::npos) {
        end_pos = url.length();
    }

    const std::string path = url.substr(start_pos, end_pos - start_pos);
    if (path.find(':') == std::string::npos) {
        return std::nullopt;
    }

    return proxy::remote_info::parse(path);
}

} // namespace

std::unique_ptr<model> proxy_model_sub_resolver::resolve(const std::filesystem::path& base, const std::string& uri)
{
    if (uri.rfind("proxyfmu", 0) == 0) {
        const auto find = uri.find("file=", 0);
        if (find == std::string::npos) {
            throw std::runtime_error("proxyfmu source missing file= component..");
        }
        const auto fmuFile = base / uri.substr(find + 5);
        std::optional<proxy::remote_info> info = parse(uri);
        return std::make_unique<proxy_model>(fmuFile, info);
    }
    return nullptr;
}
