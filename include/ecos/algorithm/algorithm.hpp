
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

    virtual double step(double currentTime, const std::vector<std::unique_ptr<model_instance>>& instances) = 0;

    virtual ~algorithm() = default;
};


} // namespace ecos

#endif // ECOS_ALGORITHM_HPP
