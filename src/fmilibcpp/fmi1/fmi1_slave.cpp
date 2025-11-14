
#include "fmi1_slave.hpp"

#include "ecos/logger/logger.hpp"

#include <fmi4c.h>

#include <cstdarg>
#include <iostream>
#include <sstream>
#include <utility>

namespace
{

const char* fmi1StatusToString(fmi1Status status)
{
    switch (status) {
        case fmi1OK: return "OK";
        case fmi1Warning: return "Warning";
        case fmi1Discard: return "Discard";
        case fmi1Error: return "Error";
        case fmi1Fatal: return "Fatal";
        case fmi1Pending: return "Pending";
        default: return "Unknown";
    }
}

void fmilogger(fmi1Component* /*c*/, fmi1String instanceName, fmi1Status status, fmi1String /*category*/, fmi1String message, ...)
{
    va_list args;
    va_start(args, message);
    char formatted[1024];
    vsnprintf(formatted, sizeof(formatted), message, args);
    va_end(args);

    std::ostringstream ss;
    ss << "[" << instanceName << "] " << fmi1StatusToString(status) << " " << formatted << "\n";

    ecos::log::debug(ss.str());
}

void noopfmilogger(fmi1Component*, fmi1String, fmi1Status, fmi1String, fmi1String, ...)
{
}

} // namespace


namespace fmilibcpp
{

fmi1_slave::fmi1_slave(
    const std::shared_ptr<fmicontext>& ctx,
    const std::string& instanceName,
    model_description md)
    : slave(instanceName)
    , ctx_(ctx)
    , md_(std::move(md))
{

    component_ = fmi1_instantiateSlave(
        ctx_->get(),
        "application/x-fmu-sharedlibrary",
        1000,
        fmi1False,
        fmi1False,
        fmilogger,
        std::calloc, std::free,
        nullptr,
        fmi1False);

    if (!component_) {
        fmi1_slave::freeInstance();
        throw std::runtime_error(std::string("Failed to instantiate fmi1 slave!"));
    }
}

const model_description& fmi1_slave::get_model_description() const
{
    return md_;
}

void fmi1_slave::set_debug_logging(bool flag)
{
    fmi1_setDebugLogging(component_, flag ? fmi1True : fmi1False);
}

bool fmi1_slave::enter_initialization_mode(double start_time, double stop_time, double /*tolerance*/)
{
    start_time_ = start_time;
    stop_time_ = stop_time;

    return true;
}

bool fmi1_slave::exit_initialization_mode()
{
    const fmi1Boolean stop_defined = (stop_time_ > 0) ? fmi1True : fmi1False;
    const auto status = fmi1_initializeSlave(component_, start_time_, stop_defined, stop_time_);
    return status == fmi1OK;
}

bool fmi1_slave::step(double current_time, double step_size)
{
    const auto status = fmi1_doStep(component_, current_time, step_size, fmi1True);
    return status == fmi1OK;
}

bool fmi1_slave::terminate()
{
    const auto status = fmi1_terminateSlave(component_);
    return status == fmi1OK;
}

bool fmi1_slave::reset()
{
    const auto status = fmi1_resetSlave(component_);
    return status == fmi1OK;
}

bool fmi1_slave::get_integer(const std::vector<value_ref>& vr, std::vector<int32_t>& values)
{
    const auto status = fmi1_getInteger(component_, vr.data(), vr.size(), values.data());
    return status == fmi1OK;
}

bool fmi1_slave::get_real(const std::vector<value_ref>& vr, std::vector<double>& values)
{
    const auto status = fmi1_getReal(component_, vr.data(), vr.size(), values.data());
    return status == fmi1OK;
}

bool fmi1_slave::get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values)
{
    auto tmp = std::vector<fmi1String>(vr.size());
    const auto status = fmi1_getString(component_, vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i];
    }
    return status == fmi1OK;
}

bool fmi1_slave::get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values)
{
    auto tmp = std::vector<fmi1Boolean>(vr.size());
    const auto status = fmi1_getBoolean(component_, vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i] != 0;
    }
    return status == fmi1OK;
}

bool fmi1_slave::set_integer(const std::vector<value_ref>& vr, const std::vector<int32_t>& values)
{
    const auto status = fmi1_setInteger(component_, vr.data(), vr.size(), values.data());
    return status == fmi1OK;
}

bool fmi1_slave::set_real(const std::vector<value_ref>& vr, const std::vector<double>& values)
{
    const auto status = fmi1_setReal(component_, vr.data(), vr.size(), values.data());
    return status == fmi1OK;
}

bool fmi1_slave::set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values)
{
    std::vector<fmi1String> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i].c_str();
    }
    const auto status = fmi1_setString(component_, vr.data(), vr.size(), _values.data());
    return status == fmi1OK;
}

bool fmi1_slave::set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values)
{
    std::vector<fmi1Boolean> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i] ? fmi1True : fmi1False;
    }
    const auto status = fmi1_setBoolean(component_, vr.data(), vr.size(), _values.data());
    return status == fmi1OK;
}

void fmi1_slave::freeInstance()
{
    if (!freed_) {
        freed_ = true;
        fmi1_freeSlaveInstance(component_);
    }
}

fmi1_slave::~fmi1_slave()
{
    fmi1_slave::freeInstance();
}

} // namespace fmilibcpp
