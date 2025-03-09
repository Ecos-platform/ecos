
#include "ecos/model_resolver.hpp"

#include "ecos/resolvers/file_model_sub_resolver.hpp"
#ifdef ECOS_WITH_PROXYFMU
#include "ecos/resolvers/proxy_model_sub_resolver.hpp"
#endif
#include "ecos/resolvers/url_model_sub_resolver.hpp"
#include "ecos/logger/logger.hpp"

using namespace ecos;

std::unique_ptr<model_resolver> ecos::default_model_resolver()
{
    auto resolver = std::make_unique<model_resolver>();
    resolver->add_sub_resolver(std::make_unique<file_model_sub_resolver>());
#ifdef ECOS_WITH_PROXYFMU
    resolver->add_sub_resolver(std::make_unique<proxy_model_sub_resolver>());
#endif
    // resolver->add_sub_resolver(std::make_unique<url_model_sub_resolver>());
    return resolver;
}

std::shared_ptr<model> model_resolver::resolve(const std::filesystem::path& base, const std::string& uri)
{
    std::string key = base.string() + "::" + uri;
    if (cache_.contains(key)) {
        log::debug("Resolver cache hit for key {}", key);
        return cache_.at(key);
    }
    for (const auto& resolver : subResolvers_) {
        if (std::shared_ptr model = resolver->resolve(base, uri)) {
            cache_[key] = model;
            return model;
        }
    }
    log::warn("No registered resolvers able to resolve uri: {}", uri);
    return nullptr;
}
