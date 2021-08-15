
#ifndef VICO_SLAVE_HPP
#define VICO_SLAVE_HPP

#include "vico/fmi/model_description.hpp"

#include <memory>
#include <vector>

namespace vico
{

using value_ref = unsigned int;

class slave
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
    virtual void freeInstance() = 0;

    virtual bool get_integer(const std::vector<value_ref>& vr, std::vector<int>& values) = 0;
    virtual bool get_real(const std::vector<value_ref>& vr, std::vector<double>& values) = 0;
    virtual bool get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values) = 0;
    virtual bool get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values) = 0;

    virtual bool set_integer(const std::vector<value_ref>& vr, const std::vector<int>& values) = 0;
    virtual bool set_real(const std::vector<value_ref>& vr, const std::vector<double>& values) = 0;
    virtual bool set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values) = 0;
    virtual bool set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values) = 0;

    virtual ~slave() = default;
};

} // namespace vico

#endif // VICO_SLAVE_HPP
