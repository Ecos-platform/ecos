
#ifndef PROXY_FMU_PROXY_SLAVE_HPP
#define PROXY_FMU_PROXY_SLAVE_HPP

#include "remote_info.hpp"

#include "fmilibcpp/slave.hpp"
#include <simple_socket/SimpleConnection.hpp>
#include <simple_socket/SocketContext.hpp>

#include <filesystem>
#include <optional>
#include <thread>

namespace ecos::proxy
{

class proxy_slave : public fmilibcpp::slave
{

public:
    proxy_slave(
        const std::filesystem::path& fmuPath,
        const std::string& instanceName,
        fmilibcpp::model_description modelDescription,
        const std::optional<remote_info>& remote);

    [[nodiscard]] const fmilibcpp::model_description& get_model_description() const override;

    bool setup_experiment(double start_time, double stop_time, double tolerance) override;
    bool enter_initialization_mode() override;
    bool exit_initialization_mode() override;

    bool step(double current_time, double step_size) override;

    bool reset() override;
    bool terminate() override;
    void freeInstance() override;

    bool get_integer(const std::vector<fmilibcpp::value_ref>& vr, std::vector<int>& values) override;
    bool get_real(const std::vector<fmilibcpp::value_ref>& vr, std::vector<double>& values) override;
    bool get_string(const std::vector<fmilibcpp::value_ref>& vr, std::vector<std::string>& values) override;
    bool get_boolean(const std::vector<fmilibcpp::value_ref>& vr, std::vector<bool>& values) override;

    bool set_integer(const std::vector<fmilibcpp::value_ref>& vr, const std::vector<int>& values) override;
    bool set_real(const std::vector<fmilibcpp::value_ref>& vr, const std::vector<double>& values) override;
    bool set_string(const std::vector<fmilibcpp::value_ref>& vr, const std::vector<std::string>& values) override;
    bool set_boolean(const std::vector<fmilibcpp::value_ref>& vr, const std::vector<bool>& values) override;

    ~proxy_slave() override;


private:
    fmilibcpp::model_description modelDescription_;

    std::unique_ptr<simple_socket::SocketContext> ctx_;
    std::unique_ptr<simple_socket::SimpleConnection> client_;
    std::thread thread_;

    bool freed = false;
};

} // namespace ecos::proxy

#endif // PROXY_FMU_PROXY_SLAVE_HPP
