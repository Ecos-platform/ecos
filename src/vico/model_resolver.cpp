
#include "vico/model_resolver.hpp"
#include "vico/fmi/fmi_model_sub_resolver.hpp"
#include "vico/fmi/proxy/proxy_model_sub_resolver.hpp"

#include "spdlog/spdlog.h"

using namespace vico;

std::unique_ptr<model_resolver> vico::default_model_resolver()
{
    auto resolver = std::make_unique<model_resolver>();
    resolver->add_sub_resolver(std::make_unique<fmi_model_sub_resolver>());
    resolver->add_sub_resolver(std::make_unique<proxy_model_sub_resolver>());
    return resolver;
}

std::shared_ptr<model> model_resolver::resolve(const std::filesystem::path& base, const std::string& uri)
{
    std::string key = base.string() + "::" + uri;
    if (cache_.count(key)) {
        spdlog::debug("Resolver cache hit for key {}", key);
        return cache_.at(key);
    }
    for (auto& resolver : subResolvers_) {
        std::shared_ptr<model> model = resolver->resolve(base, uri);
        if (model) {
            cache_[key] = model;
            return model;
        }
    }
    spdlog::warn("No registered resolvers able to resolve uri: {}", uri);
    return nullptr;
}
