
#ifndef ECOS_FMI_FMI2_SLAVE_HPP
#define ECOS_FMI_FMI2_SLAVE_HPP

#include "util/temp_dir.hpp"

#include "fmilibcpp/fmicontext.hpp"
#include "fmilibcpp/slave.hpp"

#include <fmi4c.h>
#include <memory>

namespace fmilibcpp
{

class fmi2_slave : public slave
{

public:
    fmi2_slave(
        const std::shared_ptr<fmicontext>& ctx,
        const std::string& instanceName,
        model_description md);

    [[nodiscard]] const model_description& get_model_description() const override;

    void set_debug_logging(bool flag) override;

    bool enter_initialization_mode(double start_time, double stop_time, double tolerance) override;
    bool exit_initialization_mode() override;
    bool step(double current_time, double step_size) override;

    bool terminate() override;
    bool reset() override;
    void freeInstance() override;

    void* get_state() override;
    bool set_state(void* state) override;
    bool free_state(void* state) override;

    bool get_integer(const std::vector<value_ref>& vr, std::vector<int32_t>& values) override;
    bool get_real(const std::vector<value_ref>& vr, std::vector<double>& values) override;
    bool get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values) override;
    bool get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values) override;

    bool set_integer(const std::vector<value_ref>& vr, const std::vector<int32_t>& values) override;
    bool set_real(const std::vector<value_ref>& vr, const std::vector<double>& values) override;
    bool set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values) override;
    bool set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values) override;

    ~fmi2_slave() override;

private:
    bool freed_{false};
    fmi2InstanceHandle* component{nullptr};
    std::shared_ptr<fmicontext> ctx_;

    model_description md_;
};

} // namespace fmilibcpp

#endif // ECOS_FMI_FMI2_SLAVE_HPP
