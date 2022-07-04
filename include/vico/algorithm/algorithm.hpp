
#ifndef VICO_ALGORITHM_HPP
#define VICO_ALGORITHM_HPP

#include "vico/model_instance.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace vico
{

struct algorithm
{

    virtual double step(double currentTime, std::vector<std::unique_ptr<model_instance>>& instances) = 0;

    virtual ~algorithm() = default;
};


} // namespace vico

#endif // VICO_ALGORITHM_HPP
