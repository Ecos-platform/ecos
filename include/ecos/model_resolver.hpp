
#ifndef LIBECOS_MODEL_RESOLVER_HPP
#define LIBECOS_MODEL_RESOLVER_HPP

#include "ecos/model.hpp"

#include <filesystem>
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>

namespace ecos
{

class model_sub_resolver
{
public:
    virtual std::unique_ptr<model> resolve(const std::filesystem::path& base, const std::string& uri) = 0;

    virtual ~model_sub_resolver() = default;
};

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
