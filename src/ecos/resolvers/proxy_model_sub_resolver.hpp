
#ifndef LIBECOS_PROXY_MODEL_SUB_RESOLVER_HPP
#define LIBECOS_PROXY_MODEL_SUB_RESOLVER_HPP

#include "ecos/model_resolver.hpp"

namespace ecos
{

class proxy_model_sub_resolver : public model_sub_resolver
{
public:
    std::unique_ptr<model> resolve(const std::filesystem::path& base, const std::string& uri) override;
};

} // namespace ecos

#endif // LIBECOS_PROXY_MODEL_SUB_RESOLVER_HPP
