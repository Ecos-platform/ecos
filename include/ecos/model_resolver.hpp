
#ifndef LIBECOS_MODEL_RESOLVER_HPP
#define LIBECOS_MODEL_RESOLVER_HPP

#include "ecos/model.hpp"

#include <filesystem>
#include <memory>
#include <vector>

namespace ecos
{

/* *
 * \brief Interface for resolving model instances from URIs.
 *
 * This class provides a way to resolve model instances based on a base path and a URI.
 * It allows for custom resolution strategies by implementing the model_sub_resolver interface.
 */
class model_sub_resolver
{
public:
    virtual std::unique_ptr<model> resolve(const std::filesystem::path& base, const std::string& uri) = 0;

    virtual ~model_sub_resolver() = default;
};

/* *
 * \brief Model resolver for resolving model instances from URIs.
 *
 * This class manages a collection of sub-resolvers and caches resolved models.
 * It provides methods to add sub-resolvers and resolve models based on a URI.
 */
class model_resolver
{
public:
    model_resolver& add_sub_resolver(std::unique_ptr<model_sub_resolver> subResolver)
    {
        subResolvers_.emplace_back(std::move(subResolver));
        return *this;
    }

    std::shared_ptr<model> resolve(const std::string& uri)
    {
        return resolve(std::filesystem::current_path(), uri);
    }

    std::shared_ptr<model> resolve(const std::filesystem::path& base, const std::string& uri);

private:
    std::unordered_map<std::string, std::shared_ptr<model>> cache_;
    std::vector<std::unique_ptr<model_sub_resolver>> subResolvers_;
};

std::unique_ptr<model_resolver> default_model_resolver();

} // namespace ecos

#endif // LIBECOS_MODEL_RESOLVER_HPP
