
#include "fmi2_slave.hpp"

#include <fmi4c.h>

#include <cstdarg>
#include <memory>

namespace
{

void fmilogger(fmi2Component, fmi2String instanceName, fmi2Status status, fmi2String category, fmi2String message, ...)
{
    va_list args;
    va_start(args, message);
    char msgstr[1024];
    sprintf(msgstr, "%s: %s\n", category, message);
    printf(msgstr, args);
    va_end(args);
}

void noopfmilogger(fmi2Component, fmi2String, fmi2Status, fmi2String, fmi2String, ...)
{
}

} // namespace

namespace fmilibcpp
{

fmi2_slave::fmi2_slave(
    const std::shared_ptr<fmicontext>& ctx,
    const std::string& instanceName,
    model_description md,
    bool fmiLogging)
    : slave(instanceName)
    , handle_(ctx->ctx_)
    , ctx_(ctx)
    , md_(std::move(md))
{

    if (!fmi2_instantiate(handle_,
            fmi2CoSimulation,
            fmiLogging ? &fmilogger : &noopfmilogger,
            std::calloc, std::free,
            nullptr, nullptr,
            fmi2False, fmiLogging ? fmi2True : fmi2False)) {

        fmi2_slave::freeInstance();
        throw std::runtime_error(std::string("Failed to instantiate fmi2 slave! Error: ") + fmi4c_getErrorMessages());
    }
}

const model_description& fmi2_slave::get_model_description() const
{
    return md_;
}

bool fmi2_slave::setup_experiment(double start_time, double stop_time, double tolerance)
{
    fmi2Boolean stop_defined = (stop_time > 0) ? fmi2True : fmi2False;
    fmi2Boolean tolerance_defined = (tolerance > 0) ? fmi2True : fmi2False;
    const auto status = fmi2_setupExperiment(handle_, tolerance_defined, tolerance, start_time, stop_defined, stop_time);
    return status == fmi2OK;
}

bool fmi2_slave::enter_initialization_mode()
{
    const auto status = fmi2_enterInitializationMode(handle_);
    return status == fmi2OK;
}

bool fmi2_slave::exit_initialization_mode()
{
    const auto status = fmi2_exitInitializationMode(handle_);
    return status == fmi2OK;
}

bool fmi2_slave::step(double current_time, double step_size)
{
    const auto status = fmi2_doStep(handle_, current_time, step_size, fmi2True);
    return status == fmi2OK;
}

bool fmi2_slave::terminate()
{
    const auto status = fmi2_terminate(handle_);
    return status == fmi2OK;
}

bool fmi2_slave::reset()
{
    const auto status = fmi2_reset(handle_);
    return status == fmi2OK;
}

bool fmi2_slave::get_integer(const std::vector<value_ref>& vr, std::vector<int>& values)
{
    const auto status = fmi2_getInteger(handle_, vr.data(), vr.size(), values.data());
    return status == fmi2OK;
}

bool fmi2_slave::get_real(const std::vector<value_ref>& vr, std::vector<double>& values)
{
    const auto status = fmi2_getReal(handle_, vr.data(), vr.size(), values.data());
    return status == fmi2OK;
}

bool fmi2_slave::get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values)
{
    auto tmp = std::vector<fmi2String>(vr.size());
    const auto status = fmi2_getString(handle_, vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i];
    }
    return status == fmi2OK;
}

bool fmi2_slave::get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values)
{
    auto tmp = std::vector<fmi2Boolean>(vr.size());
    const auto status = fmi2_getBoolean(handle_, vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i] != 0;
    }
    return status == fmi2OK;
}

bool fmi2_slave::set_integer(const std::vector<value_ref>& vr, const std::vector<int>& values)
{
    const auto status = fmi2_setInteger(handle_, vr.data(), vr.size(), values.data());
    return status == fmi2OK;
}

bool fmi2_slave::set_real(const std::vector<value_ref>& vr, const std::vector<double>& values)
{
    const auto status = fmi2_setReal(handle_, vr.data(), vr.size(), values.data());
    return status == fmi2OK;
}

bool fmi2_slave::set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values)
{
    std::vector<fmi2String> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i].c_str();
    }
    const auto status = fmi2_setString(handle_, vr.data(), vr.size(), _values.data());
    return status == fmi2OK;
}

bool fmi2_slave::set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values)
{
    std::vector<fmi2Boolean> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i] ? fmi2True : fmi2False;
    }
    const auto status = fmi2_setBoolean(handle_, vr.data(), vr.size(), _values.data());
    return status == fmi2OK;
}

void fmi2_slave::freeInstance()
{
    if (!freed_) {
        freed_ = true;
        fmi2_freeInstance(handle_);
    }
}

fmi2_slave::~fmi2_slave()
{
    fmi2_slave::freeInstance();
}

} // namespace fmilibcpp
