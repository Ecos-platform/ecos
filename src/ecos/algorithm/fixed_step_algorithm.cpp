
#include "ecos/algorithm/fixed_step_algorithm.hpp"

using namespace ecos;

namespace
{

struct instance_wrapper
{
    int decimationFactor;
    model_instance* instance;
};

int calculateDecimationFactor(const model_instance& m, double baseStepSize)
{

    static double EPS = 1e-5;

    auto stepSizeHint = m.stepSizeHint();
    if (!stepSizeHint) return 1;

    long decimationFactor = std::max(1, static_cast<int>(std::ceil(*stepSizeHint / baseStepSize)));
    double actualStepSize = baseStepSize * decimationFactor;
    if (std::fabs(actualStepSize - *stepSizeHint) <= EPS) {
        log::warn("Actual stepSize for {} will be {} rather than requested value {}", m.instanceName(), actualStepSize, *stepSizeHint);
    }
    return decimationFactor;
}

} // namespace

class fixed_step_algorithm::impl
{

public:
    impl(double stepSize, bool parallel)
        : stepSize_(stepSize)
        , parallel_(parallel)
        , stepNumber_(0)
    { }

    void model_instance_added(model_instance* instance)
    {
        int decimationFactor = calculateDecimationFactor(*instance, stepSize_);
        instances_.emplace_back(instance_wrapper{decimationFactor, instance});
    }

    double step(double currentTime)
    {
        auto f = [currentTime, this](auto& wrapper) {
            if (stepNumber_ % wrapper.decimationFactor == 0) {
                wrapper.instance->get_properties().apply_sets();
                wrapper.instance->step(currentTime, stepSize_);
                wrapper.instance->get_properties().apply_gets();
            }
        };

        if (!parallel_) {
            std::for_each(instances_.begin(), instances_.end(), f);
        } else {
            std::for_each(std::execution::par, instances_.begin(), instances_.end(), f);
        }

        ++stepNumber_;

        return currentTime + stepSize_;
    }

    ~impl() = default;

private:
    bool parallel_;
    double stepSize_;
    size_t stepNumber_;
    std::vector<instance_wrapper> instances_;
};


fixed_step_algorithm::fixed_step_algorithm(double stepSize, bool parallel)
    : pimpl_(std::make_unique<impl>(stepSize, parallel))
{ }

void fixed_step_algorithm::model_instance_added(model_instance* instance)
{
    pimpl_->model_instance_added(instance);
}

double ecos::fixed_step_algorithm::step(double currentTime)
{
    return pimpl_->step(currentTime);
}

fixed_step_algorithm::~fixed_step_algorithm() = default;
