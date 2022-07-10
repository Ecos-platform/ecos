

#ifndef LIBVICO_FMI_MODEL_SUB_RESOLVER_HPP
#define LIBVICO_FMI_MODEL_SUB_RESOLVER_HPP

#include "vico/model_resolver.hpp"

namespace vico
{

class fmi_model_sub_resolver : public model_sub_resolver
{
public:
    std::unique_ptr<model> resolve(const std::filesystem::path& base, const std::string& uri) override;
};

} // namespace vico

#endif // LIBVICO_FMI_MODEL_SUB_RESOLVER_HPP
