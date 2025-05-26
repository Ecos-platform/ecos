
#ifndef ECOS_ALGORITHM_HPP
#define ECOS_ALGORITHM_HPP

#include "ecos/model_instance.hpp"

namespace ecos
{

/// Abstract base class for algorithms used for co-simulation orchestration.
struct algorithm
{

    virtual void model_instance_added(model_instance* instance) = 0;

    virtual double step(double currentTime) = 0;

    virtual ~algorithm() = default;
};


} // namespace ecos

#endif // ECOS_ALGORITHM_HPP
