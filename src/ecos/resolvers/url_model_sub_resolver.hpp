
#ifndef LIBECOS_URL_MODEL_SUB_RESOLVER_HPP
#define LIBECOS_URL_MODEL_SUB_RESOLVER_HPP

#include "util/temp_dir.hpp"

#include "ecos/model_resolver.hpp"

namespace ecos
{

class url_model_sub_resolver : public model_sub_resolver
{
public:
    url_model_sub_resolver();
    std::unique_ptr<model> resolve(const std::filesystem::path& base, const std::string& uri) override;

private:
    temp_dir temp_dir_;
};

} // namespace ecos

#endif // LIBECOS_URL_MODEL_SUB_RESOLVER_HPP
