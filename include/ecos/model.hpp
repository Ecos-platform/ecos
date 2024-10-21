
#ifndef ECOS_MODEL_HPP
#define ECOS_MODEL_HPP

#include "ecos/model_instance.hpp"

#include <memory>
#include <optional>

namespace ecos
{

class model
{

public:
    virtual std::unique_ptr<model_instance> instantiate(const std::string& instanceName, std::optional<double> stepSizeHint = std::nullopt) = 0;

    virtual ~model() = default;
};

} // namespace ecos

#endif // ECOS_MODEL_HPP
