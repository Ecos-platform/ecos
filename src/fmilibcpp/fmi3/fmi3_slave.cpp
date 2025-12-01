
#include "fmi3_slave.hpp"

#include <fmi4c.h>

#include <cstdarg>
#include <memory>

namespace
{

const char* fmi3StatusToString(fmi3Status status)
{
    switch (status) {
        case fmi3OK: return "OK";
        case fmi3Warning: return "Warning";
        case fmi3Discard: return "Discard";
        case fmi3Error: return "Error";
        case fmi3Fatal: return "Fatal";
        default: return "Unknown";
    }
}

void loggerFmi3(fmi3InstanceEnvironment c,
    fmi3Status status,
    fmi3String /*category*/,
    fmi3String message)
{

    const auto slave = static_cast<fmilibcpp::fmi3_slave*>(c);

    std::ostringstream ss;
    ss << "[" << slave->instanceName << "] " << fmi3StatusToString(status) << " " << message << "\n";

    ecos::log::debug(ss.str());
}

} // namespace

namespace fmilibcpp
{

fmi3_slave::fmi3_slave(
    const std::shared_ptr<fmicontext>& ctx,
    const std::string& instanceName,
    model_description md)
    : slave(instanceName)
    , ctx_(ctx)
    , md_(std::move(md))
{

    instance_ = fmi3_instantiateCoSimulation(
        ctx_->get(),
        fmi3False,
        fmi3False,
        fmi3False,
        fmi3False,
        nullptr,
        0,
        this,
        loggerFmi3,
        nullptr);

    if (!instance_) {
        fmi3_slave::freeInstance();
        throw std::runtime_error(std::string("Failed to instantiate fmi3 slave!"));
    }
}

const model_description& fmi3_slave::get_model_description() const
{
    return md_;
}

void fmi3_slave::set_debug_logging(bool flag)
{
    fmi3_setDebugLogging(instance_, flag, 0, nullptr);
}

bool fmi3_slave::enter_initialization_mode(double start_time, double stop_time, double tolerance)
{
    const auto status = fmi3_enterInitializationMode(instance_,
        tolerance > 0,
        tolerance,
        start_time,
        stop_time > 0,
        stop_time);
    return status == fmi3OK;
}

bool fmi3_slave::exit_initialization_mode()
{
    const auto status = fmi3_exitInitializationMode(instance_);
    return status == fmi3OK;
}

bool fmi3_slave::step(double current_time, double step_size)
{
    bool eventEncountered{};
    bool terminateSimulation{};
    bool earlyReturn{};
    double lastTime{};
    const auto status = fmi3_doStep(
        instance_,
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
    const auto status = fmi3_terminate(instance_);
    return status == fmi3OK;
}

bool fmi3_slave::reset()
{
    const auto status = fmi3_reset(instance_);
    return status == fmi3OK;
}

bool fmi3_slave::get_integer(const std::vector<value_ref>& vr, std::vector<int32_t>& values)
{
    fmi3Status status;
    const auto ref = fmi3_getVariableByValueReference(ctx_->get(), vr.front());
    switch (fmi3_getVariableDataType(ref)) {
        case fmi3DataTypeInt8:
            status = fmi3_getInt8(instance_, vr.data(), vr.size(), reinterpret_cast<int8_t*>(values.data()), values.size());
            break;
        case fmi3DataTypeInt16:
            status = fmi3_getInt16(instance_, vr.data(), vr.size(), reinterpret_cast<int16_t*>(values.data()), values.size());
            break;
        case fmi3DataTypeInt32:
            status = fmi3_getInt32(instance_, vr.data(), vr.size(), values.data(), values.size());
            break;
        case fmi3DataTypeInt64:
            status = fmi3_getInt64(instance_, vr.data(), vr.size(), reinterpret_cast<int64_t*>(values.data()), values.size());
            break;
        default:
            return false;
    }

    return status == fmi3OK;
}

bool fmi3_slave::get_real(const std::vector<value_ref>& vr, std::vector<double>& values)
{

    fmi3Status status;
    const auto ref = fmi3_getVariableByValueReference(ctx_->get(), vr.front());
    switch (fmi3_getVariableDataType(ref)) {
        case fmi3DataTypeFloat32:
            status = fmi3_getFloat32(instance_, vr.data(), vr.size(), reinterpret_cast<fmi3Float32*>(values.data()), values.size());
            break;
        case fmi3DataTypeFloat64:
            status = fmi3_getFloat64(instance_, vr.data(), vr.size(), values.data(), values.size());
            break;
        default:
            return false;
    }

    return status == fmi3OK;
}

bool fmi3_slave::get_string(const std::vector<value_ref>& vr, std::vector<std::string>& values)
{
    auto tmp = std::vector<fmi3String>(vr.size());
    const auto status = fmi3_getString(instance_, vr.data(), vr.size(), tmp.data(), tmp.size());
    for (auto i = 0; i < tmp.size(); i++) {
        values[i] = tmp[i];
    }
    return status == fmi3OK;
}

bool fmi3_slave::get_boolean(const std::vector<value_ref>& vr, std::vector<bool>& values)
{
    bool* tmp = new bool[values.size()];
    std::ranges::copy(values, tmp);
    const auto status = fmi3_getBoolean(instance_, vr.data(), vr.size(), tmp, values.size());
    for (auto i = 0; i < vr.size(); i++) {
        values[i] = tmp[i];
    }
    delete[] tmp;
    return status == fmi3OK;
}

bool fmi3_slave::set_integer(const std::vector<value_ref>& vr, const std::vector<int32_t>& values)
{
    fmi3Status status;
    const auto ref = fmi3_getVariableByValueReference(ctx_->get(), vr.front());
    switch (fmi3_getVariableDataType(ref)) {
        case fmi3DataTypeInt8: {
            std::vector<fmi3Int8> int8Values(values.size());
            std::ranges::transform(values, int8Values.begin(),
                [](int32_t val) { return static_cast<fmi3Int8>(val); });
            status = fmi3_setInt8(instance_, vr.data(), vr.size(), int8Values.data(), values.size());
        } break;
        case fmi3DataTypeInt16: {
            std::vector<fmi3Int16> int16Values(values.size());
            std::ranges::transform(values, int16Values.begin(),
                [](int32_t val) { return static_cast<fmi3Int16>(val); });
            status = fmi3_setInt16(instance_, vr.data(), vr.size(), int16Values.data(), values.size());
        } break;
        case fmi3DataTypeInt32: {
            status = fmi3_setInt32(instance_, vr.data(), vr.size(), values.data(), values.size());
        } break;
        case fmi3DataTypeInt64: {
            std::vector<fmi3Int64> int64Values(values.size());
            std::ranges::transform(values, int64Values.begin(),
                [](int64_t val) { return static_cast<fmi3Int16>(val); });
            status = fmi3_setInt64(instance_, vr.data(), vr.size(), int64Values.data(), values.size());
        } break;
        case fmi3DataTypeUInt8: {
            std::vector<fmi3UInt8> uInt8Values(values.size());
            std::ranges::transform(values, uInt8Values.begin(),
                [](int32_t val) { return static_cast<fmi3Int8>(val); });
            status = fmi3_setUInt8(instance_, vr.data(), vr.size(), uInt8Values.data(), values.size());
        } break;
        case fmi3DataTypeUInt16: {
            std::vector<fmi3UInt16> uInt16Values(values.size());
            std::ranges::transform(values, uInt16Values.begin(),
                [](int32_t val) { return static_cast<fmi3Int16>(val); });
            status = fmi3_setUInt16(instance_, vr.data(), vr.size(), uInt16Values.data(), values.size());
        } break;
        case fmi3DataTypeUInt32: {
            std::vector<fmi3UInt32> uInt32Values(values.size());
            std::ranges::transform(values, uInt32Values.begin(),
                [](int32_t val) { return static_cast<fmi3Int16>(val); });
            status = fmi3_setUInt32(instance_, vr.data(), vr.size(), uInt32Values.data(), values.size());
        } break;
        case fmi3DataTypeUInt64: {
            std::vector<fmi3UInt64> uInt64Values(values.size());
            std::ranges::transform(values, uInt64Values.begin(),
                [](int64_t val) { return static_cast<fmi3Int16>(val); });
            status = fmi3_setUInt64(instance_, vr.data(), vr.size(), uInt64Values.data(), values.size());
        } break;
        default:
            return false;
    }

    return status == fmi3OK;
}

bool fmi3_slave::set_real(const std::vector<value_ref>& vr, const std::vector<double>& values)
{
    fmi3Status status;
    const auto ref = fmi3_getVariableByValueReference(ctx_->get(), vr.front());
    switch (fmi3_getVariableDataType(ref)) {
        case fmi3DataTypeFloat32: {
            std::vector<fmi3Float32> float32Values(values.size());
            std::ranges::transform(values, float32Values.begin(),
                [](double val) { return static_cast<fmi3Float32>(val); });
            status = fmi3_setFloat32(instance_, vr.data(), vr.size(), float32Values.data(), float32Values.size());
        } break;
        case fmi3DataTypeFloat64: {
            status = fmi3_setFloat64(instance_, vr.data(), vr.size(), values.data(), values.size());
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
    const auto status = fmi3_setString(instance_, vr.data(), vr.size(), _values.data(), _values.size());
    return status == fmi3OK;
}

bool fmi3_slave::set_boolean(const std::vector<value_ref>& vr, const std::vector<bool>& values)
{
    bool* tmp = new bool[values.size()];
    std::ranges::copy(values, tmp);
    const auto status = fmi3_setBoolean(instance_, vr.data(), vr.size(), tmp, values.size());
    delete tmp;
    return status == fmi3OK;
}

void fmi3_slave::freeInstance()
{
    if (!freed_) {
        freed_ = true;
        fmi3_freeInstance(instance_);
    }
}

void* fmi3_slave::get_state()
{
    if (!fmi3cs_getCanGetAndSetFMUState(ctx_->get())) {
        throw std::runtime_error("This instance cannot get and set FMU state: " + instanceName);
    }
    void* state = nullptr;
    fmi3_getFMUState(instance_, &state);

    return state;
}

bool fmi3_slave::free_state(void* state)
{
    return fmi3_freeFMUState(instance_, &state) == fmi3OK;
}

bool fmi3_slave::set_state(void* state)
{
    return fmi3_setFMUState(instance_, state) == fmi3OK;
}

fmi3_slave::~fmi3_slave()
{
    fmi3_slave::freeInstance();
}

} // namespace fmilibcpp
