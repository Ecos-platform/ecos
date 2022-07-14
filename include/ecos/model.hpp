
#ifndef ECOS_MODEL_HPP
#define ECOS_MODEL_HPP

#include "model_instance.hpp"

#include <memory>

namespace ecos
{

class model
{

public:
    virtual std::unique_ptr<model_instance> instantiate(const std::string& instanceName) = 0;

    virtual ~model() = default;
};

} // namespace ecos

#endif // ECOS_MODEL_HPP
