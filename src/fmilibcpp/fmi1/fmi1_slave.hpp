
#ifndef ECOS_FMI_FMI1_SLAVE_HPP
#define ECOS_FMI_FMI1_SLAVE_HPP

#include "fmilibcpp/fmicontext.hpp"
#include "fmilibcpp/slave.hpp"
#include "util/temp_dir.hpp"

#include <fmi4c.h>

#include <memory>

namespace fmilibcpp
{

class fmi1_slave : public slave
{

public:
    fmi1_slave(
        const std::shared_ptr<fmicontext>& ctx,
        const std::string& instanceName,
        model_description md,
        bool fmiLogging);

    [[nodiscard]] const model_description& get_model_description() const override;

    bool enter_initialization_mode(double start_time, double stop_time, double tolerance) override;
    bool exit_initialization_mode() override;
    bool step(double current_time, double step_size) override;

    bool terminate() override;
    bool reset() override;
    void freeInstance() override;

    bool get_integer(const std::vector<value_ref>& vr, std::vector<int32_t>& values) override;
    bool get_real(const std::vector<value_ref>& vr, std::vector<double>& values) override;
    bool get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values) override;
    bool get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values) override;

    bool set_integer(const std::vector<value_ref>& vr, const std::vector<int32_t>& values) override;
    bool set_real(const std::vector<value_ref>& vr, const std::vector<double>& values) override;
    bool set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values) override;
    bool set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values) override;

    ~fmi1_slave() override;

private:
    bool freed_{false};
    fmi1InstanceHandle* component_{nullptr};
    std::shared_ptr<fmicontext> ctx_;

    model_description md_;

    double start_time_{};
    double stop_time_{};
};

} // namespace fmilibcpp

#endif // ECOS_FMI_FMI1_SLAVE_HPP
