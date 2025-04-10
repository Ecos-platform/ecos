
#include "fmi1_slave.hpp"

#include <fmi4c.h>

#include <cstdarg>
#include <utility>

namespace
{

void fmilogger(fmi1Component_t c, fmi1String instanceName, fmi1Status status, fmi1String category, fmi1String message, ...)
{
    va_list args;
    va_start(args, message);
    char msgstr[1024];
    sprintf(msgstr, "%s: %s\n", category, message);
    printf(msgstr, args);
    va_end(args);
}

void noopfmilogger(fmi1Component_t, fmi1String, fmi1Status, fmi1String, fmi1String, ...)
{
}

} // namespace


namespace fmilibcpp
{

fmi1_slave::fmi1_slave(
    const std::shared_ptr<fmicontext>& ctx,
    const std::string& instanceName,
    model_description md,
    bool fmiLogging)
    : slave(instanceName)
    , ctx_(ctx)
    , md_(std::move(md))
{

    component_ = fmi1_instantiateSlave(
            ctx_->handle_,
            "application/x-fmu-sharedlibrary",
            1000,
            fmi1False,
            fmi1False,
            fmiLogging ? fmilogger : noopfmilogger,
            std::calloc, std::free,
            nullptr,
            fmiLogging ? fmi1True : fmi1False);

        if (!component_) {
            fmi1_slave::freeInstance();
            throw std::runtime_error(std::string("Failed to instantiate fmi1 slave!"));
    }
}

const model_description& fmi1_slave::get_model_description() const
{
    return md_;
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
    const auto status = fmi1_initializeSlave(ctx_->handle_, component_, start_time_, stop_defined, stop_time_);
    return status == fmi1OK;
}

bool fmi1_slave::step(double current_time, double step_size)
{
    const auto status = fmi1_doStep(ctx_->handle_, component_, current_time, step_size, fmi1True);
    return status == fmi1OK;
}

bool fmi1_slave::terminate()
{
    const auto status = fmi1_terminateSlave(ctx_->handle_, component_);
    return status == fmi1OK;
}

bool fmi1_slave::reset()
{
    const auto status = fmi1_resetSlave(ctx_->handle_, component_);
    return status == fmi1OK;
}

bool fmi1_slave::get_integer(const std::vector<value_ref>& vr, std::vector<int32_t>& values)
{
    const auto status = fmi1_getInteger(ctx_->handle_, component_, vr.data(), vr.size(), values.data());
    return status == fmi1OK;
}

bool fmi1_slave::get_real(const std::vector<value_ref>& vr, std::vector<double>& values)
{
    const auto status = fmi1_getReal(ctx_->handle_, component_, vr.data(), vr.size(), values.data());
    return status == fmi1OK;
}

bool fmi1_slave::get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values)
{
    auto tmp = std::vector<fmi1String>(vr.size());
    const auto status = fmi1_getString(ctx_->handle_, component_, vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i];
    }
    return status == fmi1OK;
}

bool fmi1_slave::get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values)
{
    auto tmp = std::vector<fmi1Boolean>(vr.size());
    const auto status = fmi1_getBoolean(ctx_->handle_, component_, vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i] != 0;
    }
    return status == fmi1OK;
}

bool fmi1_slave::set_integer(const std::vector<value_ref>& vr, const std::vector<int32_t>& values)
{
    const auto status = fmi1_setInteger(ctx_->handle_, component_, vr.data(), vr.size(), values.data());
    return status == fmi1OK;
}

bool fmi1_slave::set_real(const std::vector<value_ref>& vr, const std::vector<double>& values)
{
    const auto status = fmi1_setReal(ctx_->handle_, component_, vr.data(), vr.size(), values.data());
    return status == fmi1OK;
}

bool fmi1_slave::set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values)
{
    std::vector<fmi1String> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i].c_str();
    }
    const auto status = fmi1_setString(ctx_->handle_, component_, vr.data(), vr.size(), _values.data());
    return status == fmi1OK;
}

bool fmi1_slave::set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values)
{
    std::vector<fmi1Boolean> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i] ? fmi1True : fmi1False;
    }
    const auto status = fmi1_setBoolean(ctx_->handle_, component_, vr.data(), vr.size(), _values.data());
    return status == fmi1OK;
}

void fmi1_slave::freeInstance()
{
    if (!freed_) {
        freed_ = true;
        fmi1_freeSlaveInstance(ctx_->handle_, component_);
    }
}

fmi1_slave::~fmi1_slave()
{
    fmi1_slave::freeInstance();
}

} // namespace fmilibcpp
