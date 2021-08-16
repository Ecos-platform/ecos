
#ifndef VICO_MODEL_INSTANCE_HPP
#define VICO_MODEL_INSTANCE_HPP

#include "vico/model_description.hpp"

#include <unordered_map>
#include <vector>

namespace vico
{

class model_instance
{
public:
    virtual bool setup_experiment(
        double start_time = 0,
        double stop_time = 0,
        double tolerance = 0) = 0;

    [[nodiscard]] virtual const model_description& get_model_description() const = 0;

    virtual bool enter_initialization_mode() = 0;
    virtual bool exit_initialization_mode() = 0;

    virtual bool step(double current_time, double step_size) = 0;

    virtual bool terminate() = 0;

    virtual bool get_integer(const std::vector<value_ref>& vr, std::vector<int>& values) = 0;
    virtual bool get_real(const std::vector<value_ref>& vr, std::vector<double>& values) = 0;
    virtual bool get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values) = 0;
    virtual bool get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values) = 0;

    virtual bool set_integer(const std::vector<value_ref>& vr, const std::vector<int>& values) = 0;
    virtual bool set_real(const std::vector<value_ref>& vr, const std::vector<double>& values) = 0;
    virtual bool set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values) = 0;
    virtual bool set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values) = 0;

    virtual ~model_instance() = default;
};

class buffered_model_instance : public model_instance
{

public:
    explicit buffered_model_instance(std::unique_ptr<model_instance> instance)
        : instance_(std::move(instance))
    { }

    bool setup_experiment(double start_time, double stop_time, double tolerance) override
    {
        return instance_->setup_experiment(start_time, stop_time, tolerance);
    }

    [[nodiscard]] const model_description& get_model_description() const override
    {
        return instance_->get_model_description();
    }

    bool enter_initialization_mode() override
    {
        return instance_->enter_initialization_mode();
    }

    bool exit_initialization_mode() override
    {
        return instance_->exit_initialization_mode();
    }

    bool step(double current_time, double step_size) override
    {
        return instance_->step(current_time, step_size);
    }

    bool terminate() override
    {
        return instance_->terminate();
    }

    bool get_integer(const std::vector<value_ref>& vr, std::vector<int>& values) override
    {
        return false;
    }

    bool get_real(const std::vector<value_ref>& vr, std::vector<double>& values) override
    {
        return false;
    }

    bool get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values) override
    {
        return false;
    }

    bool get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values) override
    {
        return false;
    }

    bool set_integer(const std::vector<value_ref>& vr, const std::vector<int>& values) override
    {
        return false;
    }

    bool set_real(const std::vector<value_ref>& vr, const std::vector<double>& values) override
    {
        return false;
    }

    bool set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values) override
    {
        return false;
    }

    bool set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values) override
    {
        return false;
    }

    void mark_for_reading(const std::string& variableName)
    {

        bool marked = variables_.end() != std::find_if(variables_.begin(), variables_.end(), [&variableName](const scalar_variable& mv) {
            return mv.name == variableName;
        });

        if (marked) return;

        auto md = get_model_description();
        auto v = md.get_by_name(variableName);

        if (!v) {
            throw std::runtime_error("No variable named " + variableName + " exists for model " + md.modelName);
        }

        if (v->is_integer()) {
            integersToFetch_.emplace_back(v->valueRef);
        } else if (v->is_real()) {
            realsToFetch_.emplace_back(v->valueRef);
        } else if (v->is_string()) {
            stringsToFetch_.emplace_back(v->valueRef);
        } else if (v->is_boolean()) {
            boolsToFetch_.emplace_back(v->valueRef);
        } else {
            throw std::runtime_error("TODO");
        }

        variables_.emplace_back(*v);
    }

    void transferCachedSets()
    {

        if (!integerSetCache_.empty()) {

            vrs_.resize(integerSetCache_.size());
            std::vector<int> values(integerSetCache_.size());

            for (auto& [vr, value] : integerSetCache_) {
                vrs_.emplace_back(vr);
                values.emplace_back(value);
            }
            instance_->set_integer(vrs_, values);
        }

        if (!realSetCache_.empty()) {

            vrs_.resize(realSetCache_.size());
            std::vector<double> values(realSetCache_.size());

            for (auto& [vr, value] : realSetCache_) {
                vrs_.emplace_back(vr);
                values.emplace_back(value);
            }
            instance_->set_real(vrs_, values);
        }

        if (!stringSetCache_.empty()) {

            vrs_.resize(stringSetCache_.size());
            std::vector<std::string> values(stringSetCache_.size());

            for (auto& [vr, value] : stringSetCache_) {
                vrs_.emplace_back(vr);
                values.emplace_back(value);
            }
            instance_->set_string(vrs_, values);
        }

        if (!boolSetCache_.empty()) {

            vrs_.resize(boolSetCache_.size());
            std::vector<bool> values(boolSetCache_.size());

            for (auto& [vr, value] : boolSetCache_) {
                vrs_.emplace_back(vr);
                values.emplace_back(value);
            }
            instance_->set_boolean(vrs_, values);
        }
    }

    void retreiveCachedGets()
    {
    }

private:
    std::unique_ptr<model_instance> instance_;

    std::vector<value_ref> vrs_;

    std::vector<scalar_variable> variables_;

    std::unordered_map<value_ref, int> integerSetCache_;
    std::unordered_map<value_ref, double> realSetCache_;
    std::unordered_map<value_ref, std::string> stringSetCache_;
    std::unordered_map<value_ref, bool> boolSetCache_;

    std::unordered_map<value_ref, int> integerGetCache_;
    std::unordered_map<value_ref, double> realGetCache_;
    std::unordered_map<value_ref, std::string> stringGetCache_;
    std::unordered_map<value_ref, bool> boolGetCache_;

    std::vector<value_ref> integersToFetch_;
    std::vector<value_ref> realsToFetch_;
    std::vector<value_ref> stringsToFetch_;
    std::vector<value_ref> boolsToFetch_;
};

} // namespace vico

#endif // VICO_MODEL_INSTANCE_HPP
