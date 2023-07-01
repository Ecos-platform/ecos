
#include "handlers/fmu_service_handler.hpp"

#include <iostream>
#include <utility>

using namespace proxyfmu::thrift;
using namespace proxyfmu::server;

fmu_service_handler::fmu_service_handler(
    const std::string& fmuPath,
    const std::string& instanceName,
    std::function<void()> stop)
    : fmuPath_(fmuPath)
    , instanceName_(instanceName)
    , stop_(std::move(stop))
{
}

void fmu_service_handler::instantiate()
{
    auto fmu = fmilibcpp::loadFmu(fmuPath_);
    modelName_ = fmu->get_model_description().modelName;
    slave_ = fmu->new_instance(instanceName_);
}

Status::type fmu_service_handler::setup_experiment(const double start, const double stop, const double tolerance)
{
    auto status = slave_->setup_experiment(start, stop, tolerance);
    return status ? Status::OK_STATUS : Status::ERROR_STATUS;
}

Status::type fmu_service_handler::enter_initialization_mode()
{
    auto status = slave_->enter_initialization_mode();
    return status ? Status::OK_STATUS : Status::ERROR_STATUS;
}

Status::type fmu_service_handler::exit_initialization_mode()
{
    auto status = slave_->exit_initialization_mode();
    return status ? Status::OK_STATUS : Status::ERROR_STATUS;
}
Status::type fmu_service_handler::step(const double currentTime, const double stepSize)
{
    auto status = slave_->step(currentTime, stepSize);
    return status ? Status::OK_STATUS : Status::ERROR_STATUS;
}

Status::type fmu_service_handler::terminate()
{
    auto status = slave_->terminate();
    return status ? Status::OK_STATUS : Status::ERROR_STATUS;
}

Status::type fmu_service_handler::reset()
{
    auto status = slave_->reset();
    return status ? Status::OK_STATUS : Status::ERROR_STATUS;
}

void fmu_service_handler::read_integer(IntegerRead& _return, const ValueReferences& vr)
{
    const std::vector<fmilibcpp::value_ref> _vr(vr.begin(), vr.end());
    _return.value.resize(vr.size());
    auto status = slave_->get_integer(_vr, _return.value);
    _return.status = status ? Status::OK_STATUS : Status::ERROR_STATUS;
}

void fmu_service_handler::read_real(RealRead& _return, const ValueReferences& vr)
{
    const std::vector<fmilibcpp::value_ref> _vr(vr.begin(), vr.end());
    _return.value.resize(vr.size());
    auto status = slave_->get_real(_vr, _return.value);
    _return.status = status ? Status::OK_STATUS : Status::ERROR_STATUS;
}


void fmu_service_handler::read_string(StringRead& _return, const ValueReferences& vr)
{
    const std::vector<fmilibcpp::value_ref> _vr(vr.begin(), vr.end());
    _return.value.resize(vr.size());
    auto status = slave_->get_string(_vr, _return.value);
    _return.status = status ? Status::OK_STATUS : Status::ERROR_STATUS;
}

void fmu_service_handler::read_boolean(BooleanRead& _return, const ValueReferences& vr)
{
    const std::vector<fmilibcpp::value_ref> _vr(vr.begin(), vr.end());
    _return.value.resize(vr.size());
    auto status = slave_->get_boolean(_vr, _return.value);
    _return.status = status ? Status::OK_STATUS : Status::ERROR_STATUS;
}

Status::type fmu_service_handler::write_integer(const ValueReferences& vr, const IntArray& value)
{
    const std::vector<fmilibcpp::value_ref> _vr(vr.begin(), vr.end());
    auto status = slave_->set_integer(_vr, value);
    return status ? Status::OK_STATUS : Status::ERROR_STATUS;
}

Status::type fmu_service_handler::write_real(const ValueReferences& vr, const RealArray& value)
{
    const std::vector<fmilibcpp::value_ref> _vr(vr.begin(), vr.end());
    auto status = slave_->set_real(_vr, value);
    return status ? Status::OK_STATUS : Status::ERROR_STATUS;
}

Status::type fmu_service_handler::write_string(const ValueReferences& vr, const StringArray& value)
{
    const std::vector<fmilibcpp::value_ref> _vr(vr.begin(), vr.end());
    auto status = slave_->set_string(_vr, value);
    return status ? Status::OK_STATUS : Status::ERROR_STATUS;
}


Status::type fmu_service_handler::write_boolean(const ValueReferences& vr, const BooleanArray& value)
{
    const std::vector<fmilibcpp::value_ref> _vr(vr.begin(), vr.end());
    auto status = slave_->set_boolean(_vr, value);
    return status ? Status::OK_STATUS : Status::ERROR_STATUS;
}


void fmu_service_handler::freeInstance()
{
    slave_->freeInstance();
    std::cout << "[proxyfmu] Shutting down proxy for '" << modelName_ << "::" << instanceName_ << "'";
    stop_();
    std::cout << " done.." << std::endl;
    std::cout << "[proxyfmu] freed";
}
