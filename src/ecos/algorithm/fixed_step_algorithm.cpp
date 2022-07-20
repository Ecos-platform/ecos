
#include "ecos/algorithm/fixed_step_algorithm.hpp"

using namespace ecos;

double fixed_step_algorithm::step(double currentTime, const std::vector<std::unique_ptr<model_instance>>& instances)
{

    auto f = [currentTime, this](auto& instance) {
        instance->get_properties().apply_sets();
        instance->step(currentTime, stepSize_);
        instance->get_properties().apply_gets();
    };

    if (!parallel_) {
        std::for_each(instances.begin(), instances.end(), f);
    } else {
        std::for_each(std::execution::par, instances.begin(), instances.end(), f);
    }

    return currentTime + stepSize_;
}
