
#include "fmi1_slave.hpp"

#include "../../../cmake-build-debug-wsl/_deps/fmi4c-src/src/fmi4c_private.h"
#include "fmi4c.h"

#include <fmi4c.h>
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

void noopfmilogger(fmi1Component_t , fmi1String , fmi1Status , fmi1String , fmi1String , ...)
{
}

} // namespace


namespace fmilibcpp
{

fmi1_slave::fmi1_slave(
    const std::shared_ptr<fmicontext>& ctx,
    const std::string& instanceName,
    model_description md,
    std::shared_ptr<ecos::temp_dir> tmpDir,
    bool fmiLogging)
    : slave(instanceName)
    , handle_(fmi1_import_parse_xml(ctx->ctx_, tmpDir->path().string().c_str()))
    , md_(std::move(md))
    , ctx_(ctx)
    , tmpDir_(std::move(tmpDir))
{

    fmi1CallbackLogger_t logger = fmiLogging ? fmilogger : noopfmilogger;
    if (!fmi1_instantiateModel(handle_.get(), logger, std::calloc, std::free, fmiLogging ? fmi1True : fmi1False)) {
        fmi1_freeModelInstance(handle_.get());
        throw std::runtime_error(std::string("failed to load fmu dll! Error: ") + fmi4cErrorMessage(handle_));
    }

    const auto rc = fmi1_instantiateSlave(
        handle_.get(),
        "application/x-fmu-sharedlibrary",
        1000,
        fmi1False,
        fmi1False,
        logger,
        std::calloc,
        std::free,
        nullptr,
        fmi1True);

    if (!rc) {

        fmi1_freeSlaveInstance(handle_.get());
        throw std::runtime_error("failed to instantiate slave!");
    }
}

const model_description& fmi1_slave::get_model_description() const
{
    return md_;
}

bool fmi1_slave::setup_experiment(double start_time, double stop_time, double /*tolerance*/)
{
    start_time_ = start_time;
    stop_time_ = stop_time;
    return true;
}

bool fmi1_slave::enter_initialization_mode()
{
    return true;
}

bool fmi1_slave::exit_initialization_mode()
{
    const fmi1Boolean stop_defined = (stop_time_ > 0) ? fmi1True : fmi1False;
    const auto status = fmi1_initializeSlave(handle_.get(), start_time_, stop_defined, stop_time_);
    return status == fmi1OK;
}

bool fmi1_slave::step(double current_time, double step_size)
{
    const auto status = fmi1_doStep(handle_.get(), current_time, step_size, fmi1True);
    return status == fmi1OK;
}

bool fmi1_slave::terminate()
{
    const auto status = fmi1_terminateSlave(handle_.get());
    return status == fmi1OK;
}

bool fmi1_slave::reset()
{
    const auto status = fmi1_resetSlave(handle_.get());
    return status == fmi1OK;
}

bool fmi1_slave::get_integer(const std::vector<value_ref>& vr, std::vector<int>& values)
{
    const auto status = fmi1_getInteger(handle_.get(), vr.data(), vr.size(), values.data());
    return status == fmi1OK;
}

bool fmi1_slave::get_real(const std::vector<value_ref>& vr, std::vector<double>& values)
{
    const auto status = fmi1_getReal(handle_.get(), vr.data(), vr.size(), values.data());
    return status == fmi1OK;
}

bool fmi1_slave::get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values)
{
    auto tmp = std::vector<fmi1String>(vr.size());
    const auto status = fmi1_getString(handle_.get(), vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i];
    }
    return status == fmi1OK;
}

bool fmi1_slave::get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values)
{
    auto tmp = std::vector<fmi1Boolean>(vr.size());
    const auto status = fmi1_getBoolean(handle_.get(), vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i] != 0;
    }
    return status == fmi1OK;
}

bool fmi1_slave::set_integer(const std::vector<value_ref>& vr, const std::vector<int>& values)
{
    const auto status = fmi1_setInteger(handle_.get(), vr.data(), vr.size(), values.data());
    return status == fmi1OK;
}

bool fmi1_slave::set_real(const std::vector<value_ref>& vr, const std::vector<double>& values)
{
    const auto status = fmi1_setReal(handle_.get(), vr.data(), vr.size(), values.data());
    return status == fmi1OK;
}

bool fmi1_slave::set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values)
{
    std::vector<fmi1String> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i].c_str();
    }
    const auto status = fmi1_setString(handle_.get(), vr.data(), vr.size(), _values.data());
    return status == fmi1OK;
}

bool fmi1_slave::set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values)
{
    std::vector<fmi1Boolean> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i] ? fmi1True : fmi1False;
    }
    const auto status = fmi1_setBoolean(handle_.get(), vr.data(), vr.size(), _values.data());
    return status == fmi1OK;
}

void fmi1_slave::freeInstance()
{
    if (!freed_) {
        freed_ = true;
        fmi1_freeSlaveInstance(handle_.get());
        fmi1_freeModelInstance(handle_.get());
        // fmi1_import_free(handle_);
    }
}

fmi1_slave::~fmi1_slave()
{
    fmi1_slave::freeInstance();
}

} // namespace fmilibcpp
