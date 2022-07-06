
#include "vico/algorithm/fixed_step_algorithm.hpp"

using namespace vico;

double fixed_step_algorithm::step(double currentTime, std::vector<std::unique_ptr<model_instance>>& instances)
{

    auto f = [currentTime, this](auto& instance) {
        instance->get_properties().applySets();
        instance->step(currentTime, stepSize_);
        instance->get_properties().applyGets();
    };

    if (!parallel_) {
        std::for_each(instances.begin(), instances.end(), f);
    } else {
        std::for_each(std::execution::par, instances.begin(), instances.end(), f);
    }

    return currentTime + stepSize_;
}
