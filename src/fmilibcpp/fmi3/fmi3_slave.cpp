
#include "fmi3_slave.hpp"

#include <fmi4c.h>

#include <cstdarg>
#include <memory>

namespace
{

void loggerFmi3(fmi3InstanceEnvironment instanceEnvironment,
                 fmi3Status status,
                 fmi3String category,
                 fmi3String message)
{
    printf("%s: %s\n",category, message);
}

} // namespace

namespace fmilibcpp
{

fmi3_slave::fmi3_slave(
    const std::shared_ptr<fmicontext>& ctx,
    const std::string& instanceName,
    model_description md,
    bool fmiLogging)
    : slave(instanceName)
    , handle_(ctx->ctx_)
    , ctx_(ctx)
    , md_(std::move(md))
{

    if (!fmi3_instantiateCoSimulation(handle_,
            fmi3False,
            fmiLogging,
            fmi3False,
            fmi3False,
            nullptr,
            0,
            nullptr,
            loggerFmi3,
            nullptr)) {

        fmi3_slave::freeInstance();
        throw std::runtime_error(std::string("Failed to instantiate fmi3 slave! Error: ") + fmi4c_getErrorMessages());
    }
}

const model_description& fmi3_slave::get_model_description() const
{
    return md_;
}

bool fmi3_slave::enter_initialization_mode(double start_time, double stop_time, double tolerance)
{
    const auto status = fmi3_enterInitializationMode(handle_,
        tolerance > 0,
        tolerance,
        start_time,
        stop_time > 0,
        stop_time);
    return status == fmi3OK;
}

bool fmi3_slave::exit_initialization_mode()
{
    const auto status = fmi3_exitInitializationMode(handle_);
    return status == fmi3OK;
}

bool fmi3_slave::step(double current_time, double step_size)
{
    bool eventEncountered{};
    bool terminateSimulation{};
    bool earlyReturn{};
    double lastTime{};
    const auto status = fmi3_doStep(
        handle_,
        current_time,
        step_size,
        fmi3True,
        &eventEncountered,
        &terminateSimulation,
        &earlyReturn,
        &lastTime);
    return status == fmi3OK;
}

bool fmi3_slave::terminate()
{
    const auto status = fmi3_terminate(handle_);
    return status == fmi3OK;
}

bool fmi3_slave::reset()
{
    const auto status = fmi3_reset(handle_);
    return status == fmi3OK;
}

bool fmi3_slave::get_integer(const std::vector<value_ref>& vr, std::vector<int32_t>& values)
{
    fmi3Status status;
    const auto ref = fmi3_getVariableByValueReference(handle_, vr.front());
    switch (fmi3_getVariableDataType(ref)) {
        case fmi3DataTypeInt8:
            status = fmi3_getInt8(handle_, vr.data(), vr.size(), reinterpret_cast<int8_t*>(values.data()), values.size());
        break;
        case fmi3DataTypeInt16:
            status = fmi3_getInt16(handle_, vr.data(), vr.size(), reinterpret_cast<int16_t*>(values.data()), values.size());
        break;
        case fmi3DataTypeInt32:
            status = fmi3_getInt32(handle_, vr.data(), vr.size(), values.data(), values.size());
        break;
        case fmi3DataTypeInt64:
            status = fmi3_getInt64(handle_, vr.data(), vr.size(), reinterpret_cast<int64_t*>(values.data()), values.size());
        break;
        default:
            return false;
    }

    return status == fmi3OK;
}

bool fmi3_slave::get_real(const std::vector<value_ref>& vr, std::vector<double>& values)
{

    fmi3Status status;
    const auto ref = fmi3_getVariableByValueReference(handle_, vr.front());
    switch (fmi3_getVariableDataType(ref)) {
        case fmi3DataTypeFloat32:
            status = fmi3_getFloat32(handle_, vr.data(), vr.size(), reinterpret_cast<fmi3Float32*>(values.data()), values.size());
            break;
        case fmi3DataTypeFloat64:
            status = fmi3_getFloat64(handle_, vr.data(), vr.size(), values.data(), values.size());
            break;
        default:
            return false;
    }

    return status == fmi3OK;
}

bool fmi3_slave::get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values)
{
    auto tmp = std::vector<fmi3String>(vr.size());
    const auto status = fmi3_getString(handle_, vr.data(), vr.size(), tmp.data(), tmp.size());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i];
    }
    return status == fmi3OK;
}

bool fmi3_slave::get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values)
{
    bool* tmp = new bool[values.size()];
    std::ranges::copy(values, tmp);
    const auto status = fmi3_getBoolean(handle_, vr.data(), vr.size(), tmp, values.size());
    for (auto i = 0; i < vr.size(); i++) {
        values[i] = tmp[i];
    }
    return status == fmi3OK;
}

bool fmi3_slave::set_integer(const std::vector<value_ref>& vr, const std::vector<int>& values)
{
    // const auto status = fmi3_setInteger(handle_, vr.data(), vr.size(), values.data());
    // return status == fmi3OK;
    return false;
}

bool fmi3_slave::set_real(const std::vector<value_ref>& vr, const std::vector<double>& values)
{
    fmi3Status status;
    const auto ref = fmi3_getVariableByValueReference(handle_, vr.front());
    switch (fmi3_getVariableDataType(ref)) {
        case fmi3DataTypeFloat32: {
            std::vector<fmi3Float32> float32Values(values.size());
            std::ranges::transform(values, float32Values.begin(),
                [](double val) { return static_cast<fmi3Float32>(val); });
            status = fmi3_setFloat32(handle_, vr.data(), vr.size(), float32Values.data(), float32Values.size());
        } break;
        case fmi3DataTypeFloat64: {
            status = fmi3_setFloat64(handle_, vr.data(), vr.size(), const_cast<double*>(values.data()), values.size());
        } break;
        default:
            return false;
    }
    return status == fmi3OK;
}

bool fmi3_slave::set_string(const std::vector<value_ref>& vr, const std::vector<std::string>& values)
{
    std::vector<fmi3String> _values(vr.size());
    for (auto i = 0; i < vr.size(); i++) {
        _values[i] = values[i].c_str();
    }
    const auto status = fmi3_setString(handle_, vr.data(), vr.size(), _values.data(), _values.size());
    return status == fmi3OK;
}

bool fmi3_slave::set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values)
{
    bool* tmp = new bool[values.size()];
    std::ranges::copy(values, tmp);
    const auto status = fmi3_setBoolean(handle_, vr.data(), vr.size(), tmp, values.size());
    delete tmp;
    return status == fmi3OK;
}

void fmi3_slave::freeInstance()
{
    if (!freed_) {
        freed_ = true;
        fmi3_freeInstance(handle_);
    }
}

fmi3_slave::~fmi3_slave()
{
    fmi3_slave::freeInstance();
}

} // namespace fmilibcpp
