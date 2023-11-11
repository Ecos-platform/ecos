
#ifndef ECOS_FMI_BUFFERED_SLAVE_HPP
#define ECOS_FMI_BUFFERED_SLAVE_HPP

#include "slave.hpp"

#include <set>
#include <unordered_map>
#include <utility>

namespace fmilibcpp
{

class buffered_slave : public slave
{

public:
    buffered_slave(std::unique_ptr<slave> instance)
        : slave(instance->instanceName)
        , slave_{std::move(instance)}
    { }

    slave* get()
    {
        return slave_.get();
    }

    [[nodiscard]] const model_description& get_model_description() const override
    {
        return slave_->get_model_description();
    }

    bool setup_experiment(
        double start_time = 0,
        double stop_time = 0,
        double tolerance = 0) override
    {
        return slave_->setup_experiment(start_time, stop_time, tolerance);
    }

    bool enter_initialization_mode() override
    {
        bool status = slave_->enter_initialization_mode();
        initialized = true;
        return status;
    }

    bool exit_initialization_mode() override
    {
        bool status = slave_->exit_initialization_mode();
        return status;
    }

    bool step(double currentTime, double stepSize) override
    {
        return slave_->step(currentTime, stepSize);
    }

    bool terminate() override
    {
        return slave_->terminate();
    }

    bool reset() override
    {
        bool status = slave_->reset();
        if (status) {
            initialized = false;
        }
        return status;
    }

    bool get_integer(const std::vector<value_ref>& vrs, std::vector<int>& values) override
    {
        for (unsigned i = 0; i < vrs.size(); i++) {
            const value_ref vr = vrs[i];
            if (std::find(integersToFetch_.begin(), integersToFetch_.end(), vr) == integersToFetch_.end()) {
                mark_for_reading(get_model_description().get_by_vr<int>(vr)->name);
            }
            values[i] = integerGetCache_.at(vr);
        }
        return true;
    }

    bool get_real(const std::vector<value_ref>& vrs, std::vector<double>& values) override
    {
        for (unsigned i = 0; i < vrs.size(); i++) {
            const value_ref vr = vrs[i];
            if (std::find(realsToFetch_.begin(), realsToFetch_.end(), vr) == realsToFetch_.end()) {
                mark_for_reading(get_model_description().get_by_vr<double>(vr)->name);
            }
            values[i] = realGetCache_.at(vr);
        }
        return true;
    }

    bool get_string(const std::vector<value_ref>& vrs, std::vector<std::string>& values) override
    {
        for (unsigned i = 0; i < vrs.size(); i++) {
            const value_ref vr = vrs[i];
            if (std::find(stringsToFetch_.begin(), stringsToFetch_.end(), vr) == stringsToFetch_.end()) {
                mark_for_reading(get_model_description().get_by_vr<std::string>(vr)->name);
            }
            values[i] = stringGetCache_.at(vr);
        }
        return true;
    }

    bool get_boolean(const std::vector<value_ref>& vrs, std::vector<bool>& values) override
    {
        for (unsigned i = 0; i < vrs.size(); i++) {
            const value_ref vr = vrs[i];
            if (std::find(booleansToFetch_.begin(), booleansToFetch_.end(), vr) == booleansToFetch_.end()) {
                mark_for_reading(get_model_description().get_by_vr<bool>(vr)->name);
            }
            values[i] = booleanGetCache_.at(vr);
        }
        return true;
    }

    bool set_integer(const std::vector<value_ref>& vrs, const std::vector<int>& values) override
    {
        for (unsigned i = 0; i < vrs.size(); i++) {
            const value_ref vr = vrs[i];
            integerSetCache_[vr] = values[i];
        }
        return true;
    }

    bool set_real(const std::vector<value_ref>& vrs, const std::vector<double>& values) override
    {
        for (unsigned i = 0; i < vrs.size(); i++) {
            const value_ref vr = vrs[i];
            realSetCache_[vr] = values[i];
        }
        return true;
    }

    bool set_string(const std::vector<value_ref>& vrs, const std::vector<std::string>& values) override
    {
        for (unsigned i = 0; i < vrs.size(); i++) {
            const value_ref vr = vrs[i];
            stringSetCache_[vr] = values[i];
        }
        return true;
    }

    bool set_boolean(const std::vector<value_ref>& vrs, const std::vector<bool>& values) override
    {
        for (unsigned i = 0; i < vrs.size(); i++) {
            const value_ref vr = vrs[i];
            boolSetCache_[vr] = values[i];
        }
        return true;
    }

    void transferCachedSets()
    {
        if (!integerSetCache_.empty()) {
            std::vector<value_ref> vrs;
            std::vector<int> values;
            for (const auto& [vr, value] : integerSetCache_) {
                vrs.emplace_back(vr);
                values.emplace_back(value);
            }
            slave_->set_integer(vrs, values);
            integerSetCache_.clear();
        }
        if (!realSetCache_.empty()) {
            std::vector<value_ref> vrs;
            std::vector<double> values;
            for (const auto& [vr, value] : realSetCache_) {
                vrs.emplace_back(vr);
                values.emplace_back(value);
            }
            slave_->set_real(vrs, values);
            realSetCache_.clear();
        }
        if (!stringSetCache_.empty()) {
            std::vector<value_ref> vrs;
            std::vector<std::string> values;
            for (const auto& [vr, value] : stringSetCache_) {
                vrs.emplace_back(vr);
                values.emplace_back(value);
            }
            slave_->set_string(vrs, values);
            stringSetCache_.clear();
        }
        if (!boolSetCache_.empty()) {
            std::vector<value_ref> vrs;
            std::vector<bool> values;
            for (const auto& [vr, value] : boolSetCache_) {
                vrs.emplace_back(vr);
                values.emplace_back(value);
            }
            slave_->set_boolean(vrs, values);
            boolSetCache_.clear();
        }
    }

    void receiveCachedGets()
    {
        if (!integersToFetch_.empty()) {
            __integerGetCache_.resize(integersToFetch_.size());
            slave_->get_integer(integersToFetch_, __integerGetCache_);

            integerGetCache_.clear();
            for (unsigned i = 0; i < integersToFetch_.size(); i++) {
                const value_ref vr = integersToFetch_[i];
                integerGetCache_[vr] = __integerGetCache_[i];
            }
        }
        if (!realsToFetch_.empty()) {
            __realGetCache_.resize(realsToFetch_.size());
            slave_->get_real(realsToFetch_, __realGetCache_);

            realGetCache_.clear();
            for (unsigned i = 0; i < realsToFetch_.size(); i++) {
                const value_ref vr = realsToFetch_[i];
                realGetCache_[vr] = __realGetCache_[i];
            }
        }
        if (!stringGetCache_.empty()) {
            __stringGetCache_.resize(stringsToFetch_.size());
            slave_->get_string(stringsToFetch_, __stringGetCache_);

            stringGetCache_.clear();
            for (unsigned i = 0; i < stringsToFetch_.size(); i++) {
                const value_ref vr = stringsToFetch_[i];
                stringGetCache_[vr] = __stringGetCache_[i];
            }
        }
        if (!booleansToFetch_.empty()) {
            __booleanGetCache_.resize(booleansToFetch_.size());
            slave_->get_boolean(booleansToFetch_, __booleanGetCache_);

            booleanGetCache_.clear();
            for (unsigned i = 0; i < booleansToFetch_.size(); i++) {
                const value_ref vr = booleansToFetch_[i];
                booleanGetCache_[vr] = __booleanGetCache_[i];
            }
        }
    }

    void mark_for_reading(const std::string& variableName)
    {

        if (marked_variables.find(variableName) != marked_variables.end()) return;

        const auto& md = slave_->get_model_description();

        const auto& v = md.get_by_name(variableName);
        if (!v) throw std::runtime_error("No such variable '" + variableName + "'!");

        const value_ref vr = v->vr;

        if (v->is_integer()) {
            integersToFetch_.emplace_back(vr);
        } else if (v->is_real()) {
            realsToFetch_.emplace_back(vr);
        } else if (v->is_string()) {
            stringsToFetch_.emplace_back(vr);
        } else if (v->is_boolean()) {
            booleansToFetch_.emplace_back(vr);
        }

        marked_variables.insert(variableName);

        if (initialized) {
            if (v->is_integer()) {
                integerGetCache_[vr] = slave_->get_integer(vr);
            } else if (v->is_real()) {
                realGetCache_[vr] = slave_->get_real(vr);
            } else if (v->is_string()) {
                stringGetCache_[vr] = slave_->get_string(vr);
            } else if (v->is_boolean()) {
                booleanGetCache_[vr] = slave_->get_boolean(vr);
            }
        }
    }

    void freeInstance() override
    {
        slave_->freeInstance();
    }

private:
    std::unique_ptr<slave> slave_;

    std::unordered_map<value_ref, int> integerSetCache_;
    std::unordered_map<value_ref, double> realSetCache_;
    std::unordered_map<value_ref, std::string> stringSetCache_;
    std::unordered_map<value_ref, bool> boolSetCache_;

    std::unordered_map<value_ref, int> integerGetCache_;
    std::unordered_map<value_ref, double> realGetCache_;
    std::unordered_map<value_ref, std::string> stringGetCache_;
    std::unordered_map<value_ref, bool> booleanGetCache_;

    std::vector<int> __integerGetCache_;
    std::vector<double> __realGetCache_;
    std::vector<std::string> __stringGetCache_;
    std::vector<bool> __booleanGetCache_;

    std::vector<value_ref> integersToFetch_;
    std::vector<value_ref> realsToFetch_;
    std::vector<value_ref> stringsToFetch_;
    std::vector<value_ref> booleansToFetch_;

    std::set<std::string> marked_variables;

    bool initialized{false};
};

} // namespace fmilibcpp

#endif // ECOS_FMI_BUFFERED_SLAVE_HPP
