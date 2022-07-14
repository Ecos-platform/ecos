
#ifndef LIBECOS_FMI_MODEL_SUB_RESOLVER_HPP
#define LIBECOS_FMI_MODEL_SUB_RESOLVER_HPP

#include "ecos/model_resolver.hpp"

namespace ecos
{

class fmi_model_sub_resolver : public model_sub_resolver
{
public:
    std::unique_ptr<model> resolve(const std::filesystem::path& base, const std::string& uri) override;
};

} // namespace ecos

#endif // LIBECOS_FMI_MODEL_SUB_RESOLVER_HPP
