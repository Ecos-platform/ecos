
#include "fmi2_slave.hpp"

#include "../../../cmake-build-debug-wsl/_deps/fmi4c-src/src/fmi4c_private.h"

#include <exception>
#include <fmilib.h>
#include <memory>

namespace
{

void fmilogger(fmi2Component, fmi2String instanceName, fmi2Status status, fmi2String category, fmi2String message, ...)
{
    va_list argp;
    va_start(argp, message);
    // fmi2_log_forwarding_v(c, instanceName, status, category, message, argp);
    va_end(argp);
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
    std::shared_ptr<ecos::temp_dir> tmpDir,
    bool fmiLogging)
    : slave(instanceName)
    , handle_(fmi2_import_parse_xml(ctx->ctx_, tmpDir->path().string().c_str(), nullptr))
    , md_(std::move(md))
    , ctx_(ctx)
    , tmpDir_(std::move(tmpDir))
{

    fmi2CallbackFunctions callbackFunctions;
    callbackFunctions.allocateMemory = calloc;
    callbackFunctions.freeMemory = free;
    if (fmiLogging) {
        callbackFunctions.logger = &fmilogger;
    } else {
        callbackFunctions.logger = &noopfmilogger;
    }
    callbackFunctions.componentEnvironment = nullptr;
    callbackFunctions.stepFinished = nullptr;

    if (!fmi2_instantiate(handle_.get(), fmi2CoSimulation, &callbackFunctions)) {
        throw std::runtime_error(std::string("failed to load fmu dll! Error: ") + fmi4cErrorMessage(handle_));
    }

    const auto rc = fmi2_instantiate(
        handle_,
        this->instanceName.c_str(),
        fmi2_cosimulation,
        nullptr,
        fmi2_false);

    if (rc != jm_status_success) {
        fmi2_import_destroy_dllfmu(handle_);
        fmi2_import_free(handle_);
        throw std::runtime_error("failed to instantiate fmu!");
    }
}

const model_description& fmi2_slave::get_model_description() const
{
    return md_;
}

bool fmi2_slave::setup_experiment(double start_time, double stop_time, double tolerance)
{
    fmi2_boolean_t stop_defined = (stop_time > 0) ? fmi2_true : fmi2_false;
    fmi2_boolean_t tolerance_defined = (tolerance > 0) ? fmi2_true : fmi2_false;
    const auto status = fmi2_import_setup_experiment(handle_, tolerance_defined, tolerance, start_time, stop_defined, stop_time);
    return status == fmi2_status_ok;
}

bool fmi2_slave::enter_initialization_mode()
{
    const auto status = fmi2_import_enter_initialization_mode(handle_);
    return status == fmi2_status_ok;
}

bool fmi2_slave::exit_initialization_mode()
{
    const auto status = fmi2_import_exit_initialization_mode(handle_);
    return status == fmi2_status_ok;
}

bool fmi2_slave::step(double current_time, double step_size)
{
    const auto status = fmi2_import_do_step(handle_, current_time, step_size, fmi2_true);
    return status == fmi2_status_ok;
}

bool fmi2_slave::terminate()
{
    const auto status = fmi2_import_terminate(handle_);
    return status == fmi2_status_ok;
}

bool fmi2_slave::reset()
{
    const auto status = fmi2_import_reset(handle_);
    return status == fmi2_status_ok;
}

bool fmi2_slave::get_integer(const std::vector<value_ref>& vr, std::vector<int>& values)
{
    const auto status = fmi2_import_get_integer(handle_, vr.data(), vr.size(), values.data());
    return status == fmi2_status_ok;
}

bool fmi2_slave::get_real(const std::vector<value_ref>& vr, std::vector<double>& values)
{
    const auto status = fmi2_import_get_real(handle_, vr.data(), vr.size(), values.data());
    return status == fmi2_status_ok;
}

bool fmi2_slave::get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values)
{
    auto tmp = std::vector<fmi2_string_t>(vr.size());
    const auto status = fmi2_import_get_string(handle_, vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i];
    }
    return status == fmi2_status_ok;
}

bool fmi2_slave::get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values)
{
    auto tmp = std::vector<fmi2_boolean_t>(vr.size());
    const auto status = fmi2_import_get_boolean(handle_, vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i] != 0;
    }
    return status == fmi2_status_ok;
}

bool fmi2_slave::set_integer(const std::vector<value_ref>& vr, const std::vector<int>& values)
{
    const auto status = fmi2_import_set_integer(handle_, vr.data(), vr.size(), values.data());
    return status == fmi2_status_ok;
}

bool fmi2_slave::set_real(const std::vector<value_ref>& vr, const std::vector<double>& values)
{
    const auto status = fmi2_import_set_real(handle_, vr.data(), vr.size(), values.data());
    return status == fmi2_status_ok;
}

bool fmi2_slave::set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values)
{
    std::vector<fmi2_string_t> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i].c_str();
    }
    const auto status = fmi2_import_set_string(handle_, vr.data(), vr.size(), _values.data());
    return status == fmi2_status_ok;
}

bool fmi2_slave::set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values)
{
    std::vector<fmi2_boolean_t> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i] ? fmi2_true : fmi2_false;
    }
    const auto status = fmi2_import_set_boolean(handle_, vr.data(), vr.size(), _values.data());
    return status == fmi2_status_ok;
}

void fmi2_slave::freeInstance()
{
    if (!freed_) {
        freed_ = true;
        fmi2_import_free_instance(handle_);
        fmi2_import_destroy_dllfmu(handle_);
        fmi2_import_free(handle_);
    }
}

fmi2_slave::~fmi2_slave()
{
    fmi2_slave::freeInstance();
}

} // namespace fmilibcpp
