
#ifndef LIBVICO_PROXY_MODEL_SUB_RESOLVER_HPP
#define LIBVICO_PROXY_MODEL_SUB_RESOLVER_HPP

#include "vico/model_resolver.hpp"

namespace vico
{

class proxy_model_sub_resolver : public model_sub_resolver
{
public:
    std::unique_ptr<model> resolve(const std::filesystem::path& base, const std::string& uri) override;
};

} // namespace vico

#endif // LIBVICO_PROXY_MODEL_SUB_RESOLVER_HPP
