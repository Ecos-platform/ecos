
#ifndef PROXY_FMU_FMUSERVICEHANDLER_HPP
#define PROXY_FMU_FMUSERVICEHANDLER_HPP

#include <proxyfmu/thrift/FmuService.h>

#include <fmilibcpp/fmu.hpp>

#include <functional>

namespace proxyfmu::server
{

class fmu_service_handler : virtual public thrift::FmuServiceIf
{

private:
    std::string modelName_;

    const std::string& fmuPath_;
    const std::string& instanceName_;

    std::unique_ptr<fmilibcpp::slave> slave_;

    std::function<void()> stop_;

public:
    fmu_service_handler(const std::string& fmuPath, const std::string& instanceName, std::function<void()> stop);

    void instantiate() override;

    thrift::Status::type setup_experiment(double start, double stop, double tolerance) override;
    thrift::Status::type enter_initialization_mode() override;
    thrift::Status::type exit_initialization_mode() override;
    thrift::Status::type step(double currentTime, double stepSize) override;
    thrift::Status::type terminate() override;
    thrift::Status::type reset() override;
    void freeInstance() override;

    void read_integer(thrift::IntegerRead& _return, const thrift::ValueReferences& vr) override;
    void read_real(thrift::RealRead& _return, const thrift::ValueReferences& vr) override;
    void read_string(thrift::StringRead& _return, const thrift::ValueReferences& vr) override;
    void read_boolean(thrift::BooleanRead& _return, const thrift::ValueReferences& vr) override;

    thrift::Status::type write_integer(const thrift::ValueReferences& vr, const thrift::IntArray& value) override;
    thrift::Status::type write_real(const thrift::ValueReferences& vr, const thrift::RealArray& value) override;
    thrift::Status::type write_string(const thrift::ValueReferences& vr, const thrift::StringArray& value) override;
    thrift::Status::type write_boolean(const thrift::ValueReferences& vr, const thrift::BooleanArray& value) override;
};

} // namespace proxyfmu::server

#endif // PROXY_FMU_FMUSERVICEHANDLER_HPP
