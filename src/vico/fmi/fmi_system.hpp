
#ifndef VICO_FMI_SYSTEM_HPP
#define VICO_FMI_SYSTEM_HPP

#include "vico/system.hpp"

#include <fmilibcpp/slave.hpp>

namespace vico
{

class fmi_system : public system
{

    void add_slave(const std::string &instanceName, std::unique_ptr<fmilibcpp::slave> slave);

    void init() override;

    void step(double currentTime, double stepSize) override;

    void terminate() override;

private:
    std::vector<std::unique_ptr<fmilibcpp::slave>> slaves_;
};

} // namespace vico

#endif // VICO_FMI_SYSTEM_HPP
