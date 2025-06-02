
#ifndef ECOS_FIXED_STEP_ALGORITHM_HPP
#define ECOS_FIXED_STEP_ALGORITHM_HPP

#include "algorithm.hpp"

#include <execution>

namespace ecos
{

/**
 * \brief Fixed-step algorithm for co-simulation orchestration.
 *
 * - Supports parallel execution of model instances.
 * - Supports multi-variate step-size for individual models.
 */
class fixed_step_algorithm : public algorithm
{

public:
    explicit fixed_step_algorithm(double stepSize, bool parallel = true);

    void model_instance_added(model_instance* instance) override;

    double step(double currentTime) override;

    ~fixed_step_algorithm() override;

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};


} // namespace ecos

#endif // ECOS_FIXED_STEP_ALGORITHM_HPP
