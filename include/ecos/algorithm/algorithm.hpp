
#ifndef ECOS_ALGORITHM_HPP
#define ECOS_ALGORITHM_HPP

#include "ecos/model_instance.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace ecos
{

struct algorithm
{

    virtual void model_instance_added(model_instance* instance) = 0;

    virtual void model_instance_removed(model_instance* instance){};

    virtual double step(double currentTime) = 0;

    virtual ~algorithm() = default;
};


} // namespace ecos

#endif // ECOS_ALGORITHM_HPP
