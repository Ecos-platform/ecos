
#include "fmi1_slave.hpp"

#include <fmilib.h>
#include <utility>

namespace
{

void fmilogger(fmi1_component_t c, fmi1_string_t instanceName, fmi1_status_t status, fmi1_string_t category, fmi1_string_t message, ...)
{
    va_list argp;
    va_start(argp, message);
    fmi1_log_forwarding_v(c, instanceName, status, category, message, argp);
    va_end(argp);
}

void noopfmilogger(fmi1_component_t, fmi1_string_t, fmi1_status_t, fmi1_string_t, fmi1_string_t, ...)
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
    , ctx_(ctx)
    , md_(std::move(md))
    , tmpDir_(std::move(tmpDir))
    , handle_(fmi1_import_parse_xml(ctx->ctx_, tmpDir->path().string().c_str()))
{
    fmi1_callback_functions_t callbackFunctions;
    callbackFunctions.allocateMemory = std::calloc;
    callbackFunctions.freeMemory = std::free;
    if (fmiLogging) {
        callbackFunctions.logger = fmilogger;
    } else {
        callbackFunctions.logger = noopfmilogger;
    }
    callbackFunctions.stepFinished = nullptr;

    if (fmi1_import_create_dllfmu(handle_, callbackFunctions, 1) != jm_status_success) {
        throw std::runtime_error(std::string("failed to load fmu dll! Error: ") + fmi1_import_get_last_error(handle_));
    }

    const auto rc = fmi1_import_instantiate_slave(
        handle_,
        this->instanceName.c_str(),
        nullptr,
        nullptr,
        0,
        fmi1_false,
        fmi1_false);

    if (rc != jm_status_success) {
        fmi1_import_destroy_dllfmu(handle_);
        fmi1_import_free(handle_);
        throw std::runtime_error("failed to instantiate fmu!");
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
    fmi1_boolean_t stop_defined = (stop_time_ > 0) ? fmi1_true : fmi1_false;
    auto status = fmi1_import_initialize_slave(handle_, start_time_, stop_defined, stop_time_);
    return status == fmi1_status_ok;
}

bool fmi1_slave::step(double current_time, double step_size)
{
    auto status = fmi1_import_do_step(handle_, current_time, step_size, fmi1_true);
    return status == fmi1_status_ok;
}

bool fmi1_slave::terminate()
{
    auto status = fmi1_import_terminate_slave(handle_);
    return status == fmi1_status_ok;
}

bool fmi1_slave::reset()
{
    auto status = fmi1_import_reset_slave(handle_);
    return status == fmi1_status_ok;
}

bool fmi1_slave::get_integer(const std::vector<value_ref>& vr, std::vector<int>& values)
{
    auto status = fmi1_import_get_integer(handle_, vr.data(), vr.size(), values.data());
    return status == fmi1_status_ok;
}

bool fmi1_slave::get_real(const std::vector<value_ref>& vr, std::vector<double>& values)
{
    auto status = fmi1_import_get_real(handle_, vr.data(), vr.size(), values.data());
    return status == fmi1_status_ok;
}

bool fmi1_slave::get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values)
{
    auto tmp = std::vector<fmi1_string_t>(vr.size());
    auto status = fmi1_import_get_string(handle_, vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i];
    }
    return status == fmi1_status_ok;
}

bool fmi1_slave::get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values)
{
    auto tmp = std::vector<fmi1_boolean_t>(vr.size());
    auto status = fmi1_import_get_boolean(handle_, vr.data(), vr.size(), tmp.data());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i] != 0;
    }
    return status == fmi1_status_ok;
}

bool fmi1_slave::set_integer(const std::vector<value_ref>& vr, const std::vector<int>& values)
{
    auto status = fmi1_import_set_integer(handle_, vr.data(), vr.size(), values.data());
    return status == fmi1_status_ok;
}

bool fmi1_slave::set_real(const std::vector<value_ref>& vr, const std::vector<double>& values)
{
    auto status = fmi1_import_set_real(handle_, vr.data(), vr.size(), values.data());
    return status == fmi1_status_ok;
}

bool fmi1_slave::set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values)
{
    std::vector<fmi1_string_t> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i].c_str();
    }
    auto status = fmi1_import_set_string(handle_, vr.data(), vr.size(), _values.data());
    return status == fmi1_status_ok;
}

bool fmi1_slave::set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values)
{
    std::vector<fmi1_boolean_t> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i] ? fmi1_true : fmi1_false;
    }
    auto status = fmi1_import_set_boolean(handle_, vr.data(), vr.size(), _values.data());
    return status == fmi1_status_ok;
}

void fmi1_slave::freeInstance()
{
    if (!freed_) {
        freed_ = true;
        fmi1_import_free_slave_instance(handle_);
        fmi1_import_destroy_dllfmu(handle_);
        fmi1_import_free(handle_);
    }
}

fmi1_slave::~fmi1_slave()
{
    fmi1_slave::freeInstance();
}

} // namespace fmilibcpp