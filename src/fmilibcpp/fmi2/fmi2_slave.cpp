
#include "fmi2_slave.hpp"

#include <fmi4c.h>

#include <cstdarg>
#include <iostream>
#include <memory>
#include <sstream>

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
    char formatted[1024];
    vsnprintf(formatted, sizeof(formatted), message, args);
    va_end(args);

    std::ostringstream ss;
    ss << "[" << instanceName << "] " << fmi2StatusToString(status) << " " << formatted << "\n";

    ecos::log::debug(ss.str());
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
    model_description md)
    : slave(instanceName)
    , ctx_(ctx)
    , md_(std::move(md))
{

    component = fmi2_instantiate(
        ctx_->get(),
        fmi2CoSimulation,
        &fmilogger,
        std::calloc, std::free,
        nullptr, nullptr,
        fmi2False, fmi2False);

    if (!component) {
        fmi2_slave::freeInstance();
        throw std::runtime_error(std::string("Failed to instantiate fmi2 slave!"));
    }
}

const model_description& fmi2_slave::get_model_description() const
{
    return md_;
}

void fmi2_slave::set_debug_logging(bool flag)
{
    fmi2_setDebugLogging(component, flag ? fmi2True : fmi2False, 0, nullptr);
}

bool fmi2_slave::enter_initialization_mode(double start_time, double stop_time, double tolerance)
{
    fmi2Boolean stop_defined = (stop_time > 0) ? fmi2True : fmi2False;
    fmi2Boolean tolerance_defined = (tolerance > 0) ? fmi2True : fmi2False;
    const auto status1 = fmi2_setupExperiment(component, tolerance_defined, tolerance, start_time, stop_defined, stop_time);

    if (!status1 == fmi2OK) {
        return false;
    }

    const auto status2 = fmi2_enterInitializationMode(component);
    return status2 == fmi2OK;
}

bool fmi2_slave::exit_initialization_mode()
{
    const auto status = fmi2_exitInitializationMode(component);
    return status == fmi2OK;
}

bool fmi2_slave::step(double current_time, double step_size)
{
    const auto status = fmi2_doStep(component, current_time, step_size, fmi2True);
    return status == fmi2OK;
}

bool fmi2_slave::terminate()
{
    const auto status = fmi2_terminate(component);
    return status == fmi2OK;
}

bool fmi2_slave::reset()
{
    const auto status = fmi2_reset(component);
    return status == fmi2OK;
}

bool fmi2_slave::get_integer(const std::vector<value_ref>& vr, std::vector<int32_t>& values)
{
    const auto status = fmi2_getInteger(component, vr.data(), vr.size(), values.data());
    return status == fmi2OK;
}

bool fmi2_slave::get_real(const std::vector<value_ref>& vr, std::vector<double>& values)
{
    const auto status = fmi2_getReal(component, vr.data(), vr.size(), values.data());
    return status == fmi2OK;
}

bool fmi2_slave::get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values)
{
    auto tmp = std::vector<fmi2String>(vr.size());
    const auto status = fmi2_getString(component, vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i];
    }
    return status == fmi2OK;
}

bool fmi2_slave::get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values)
{
    auto tmp = std::vector<fmi2Boolean>(vr.size());
    const auto status = fmi2_getBoolean(component, vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i] != 0;
    }
    return status == fmi2OK;
}

bool fmi2_slave::set_integer(const std::vector<value_ref>& vr, const std::vector<int>& values)
{
    const auto status = fmi2_setInteger(component, vr.data(), vr.size(), values.data());
    return status == fmi2OK;
}

bool fmi2_slave::set_real(const std::vector<value_ref>& vr, const std::vector<double>& values)
{
    const auto status = fmi2_setReal(component, vr.data(), vr.size(), values.data());
    return status == fmi2OK;
}

bool fmi2_slave::set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values)
{
    std::vector<fmi2String> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i].c_str();
    }
    const auto status = fmi2_setString(component, vr.data(), vr.size(), _values.data());
    return status == fmi2OK;
}

bool fmi2_slave::set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values)
{
    std::vector<fmi2Boolean> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i] ? fmi2True : fmi2False;
    }
    const auto status = fmi2_setBoolean(component, vr.data(), vr.size(), _values.data());
    return status == fmi2OK;
}

void fmi2_slave::freeInstance()
{
    if (!freed_) {
        freed_ = true;
        fmi2_freeInstance(component);
    }
}

void* fmi2_slave::get_state()
{

    if (!fmi2cs_getCanGetAndSetFMUState(ctx_->get())) {
        throw std::runtime_error("This instance cannot get and set FMU state: " + instanceName);
    }

    void* state = nullptr;
    fmi2_getFMUstate(component, &state);

    return state;
}

bool fmi2_slave::set_state(void* state)
{
    return fmi2_setFMUstate(component, state) == fmi2OK;
}

bool fmi2_slave::free_state(void* state)
{
    return fmi2_freeFMUstate(component, &state) == fmi2OK;
}

fmi2_slave::~fmi2_slave()
{
    fmi2_slave::freeInstance();
}

} // namespace fmilibcpp
