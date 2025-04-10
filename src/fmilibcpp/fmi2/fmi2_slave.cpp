
#include "fmi2_slave.hpp"

#include <fmi4c.h>

#include <cstdarg>
#include <memory>

namespace
{

const char* fmi2StatusToString(fmi2Status status)
{
    switch (status) {
        case fmi2OK: return "fmi2OK";
        case fmi2Warning: return "fmi2Warning";
        case fmi2Discard: return "fmi2Discard";
        case fmi2Error: return "fmi2Error";
        case fmi2Fatal: return "fmi2Fatal";
        case fmi2Pending: return "fmi2Pending";
        default: return "Unknown fmi2Status";
    }
}

void fmilogger(fmi2Component, fmi2String instanceName, fmi2Status status, fmi2String, fmi2String message, ...)
{
    va_list args;
    va_start(args, message);
    char msgstr[1024];
    sprintf(msgstr, "[%s] %s: %s\n", fmi2StatusToString(status), instanceName, message);
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
    , ctx_(ctx)
    , md_(std::move(md))
{

    component = fmi2_instantiate(ctx_->handle_,
        fmi2CoSimulation,
        fmiLogging ? &fmilogger : &noopfmilogger,
        std::calloc, std::free,
        nullptr, nullptr,
        fmi2False, fmiLogging ? fmi2True : fmi2False);

    if (!component) {
        fmi2_slave::freeInstance();
        throw std::runtime_error(std::string("Failed to instantiate fmi2 slave!"));
    }
}

const model_description& fmi2_slave::get_model_description() const
{
    return md_;
}

bool fmi2_slave::enter_initialization_mode(double start_time, double stop_time, double tolerance)
{
    fmi2Boolean stop_defined = (stop_time > 0) ? fmi2True : fmi2False;
    fmi2Boolean tolerance_defined = (tolerance > 0) ? fmi2True : fmi2False;
    const auto status1 = fmi2_setupExperiment(ctx_->handle_, component, tolerance_defined, tolerance, start_time, stop_defined, stop_time);

    if (!status1 == fmi2OK) {
        return false;
    }

    const auto status2 = fmi2_enterInitializationMode(ctx_->handle_, component);
    return status2 == fmi2OK;
}

bool fmi2_slave::exit_initialization_mode()
{
    const auto status = fmi2_exitInitializationMode(ctx_->handle_, component);
    return status == fmi2OK;
}

bool fmi2_slave::step(double current_time, double step_size)
{
    const auto status = fmi2_doStep(ctx_->handle_, component, current_time, step_size, fmi2True);
    return status == fmi2OK;
}

bool fmi2_slave::terminate()
{
    const auto status = fmi2_terminate(ctx_->handle_, component);
    return status == fmi2OK;
}

bool fmi2_slave::reset()
{
    const auto status = fmi2_reset(ctx_->handle_, component);
    return status == fmi2OK;
}

bool fmi2_slave::get_integer(const std::vector<value_ref>& vr, std::vector<int32_t>& values)
{
    const auto status = fmi2_getInteger(ctx_->handle_, component, vr.data(), vr.size(), values.data());
    return status == fmi2OK;
}

bool fmi2_slave::get_real(const std::vector<value_ref>& vr, std::vector<double>& values)
{
    const auto status = fmi2_getReal(ctx_->handle_, component, vr.data(), vr.size(), values.data());
    return status == fmi2OK;
}

bool fmi2_slave::get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values)
{
    auto tmp = std::vector<fmi2String>(vr.size());
    const auto status = fmi2_getString(ctx_->handle_, component, vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i];
    }
    return status == fmi2OK;
}

bool fmi2_slave::get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values)
{
    auto tmp = std::vector<fmi2Boolean>(vr.size());
    const auto status = fmi2_getBoolean(ctx_->handle_, component, vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i] != 0;
    }
    return status == fmi2OK;
}

bool fmi2_slave::set_integer(const std::vector<value_ref>& vr, const std::vector<int>& values)
{
    const auto status = fmi2_setInteger(ctx_->handle_, component, vr.data(), vr.size(), values.data());
    return status == fmi2OK;
}

bool fmi2_slave::set_real(const std::vector<value_ref>& vr, const std::vector<double>& values)
{
    const auto status = fmi2_setReal(ctx_->handle_, component, vr.data(), vr.size(), values.data());
    return status == fmi2OK;
}

bool fmi2_slave::set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values)
{
    std::vector<fmi2String> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i].c_str();
    }
    const auto status = fmi2_setString(ctx_->handle_, component, vr.data(), vr.size(), _values.data());
    return status == fmi2OK;
}

bool fmi2_slave::set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values)
{
    std::vector<fmi2Boolean> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i] ? fmi2True : fmi2False;
    }
    const auto status = fmi2_setBoolean(ctx_->handle_, component, vr.data(), vr.size(), _values.data());
    return status == fmi2OK;
}

void fmi2_slave::freeInstance()
{
    if (!freed_) {
        freed_ = true;
        fmi2_freeInstance(ctx_->handle_, component);
    }
}

void* fmi2_slave::get_state()
{

    if (!fmi2cs_getCanGetAndSetFMUState(ctx_->handle_)) {
        throw std::runtime_error("This instance cannot get and set FMU state: " + instanceName);
    }

    void* state = nullptr;
    fmi2_getFMUstate(ctx_->handle_, component, &state);

    return state;
}

bool fmi2_slave::set_state(void* state)
{
    return fmi2_setFMUstate(ctx_->handle_, component, state) == fmi2OK;
}

bool fmi2_slave::free_state(void* state)
{
    return fmi2_freeFMUstate(ctx_->handle_, component, &state) == fmi2OK;
}

fmi2_slave::~fmi2_slave()
{
    fmi2_slave::freeInstance();
}

} // namespace fmilibcpp
