
#ifndef VICO_FMI_SYSTEM_HPP
#define VICO_FMI_SYSTEM_HPP

#include "vico/fmi/algorithm.hpp"
#include "vico/system.hpp"

#include <fmilibcpp/slave.hpp>

namespace vico
{

class fmi_system : public system
{

public:
    explicit fmi_system(std::unique_ptr<algorithm> algorithm);

    fmi_system(const fmi_system&) = delete;
    fmi_system(const fmi_system&&) = delete;

    void add_slave(std::unique_ptr<fmilibcpp::slave> slave);

    void init(double startTime) override;

    void step(double currentTime, double stepSize) override;

    void terminate() override;

    ~fmi_system() override;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace vico

#endif // VICO_FMI_SYSTEM_HPP
