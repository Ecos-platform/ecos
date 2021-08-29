
#ifndef VICO_FMI_SYSTEM_HPP
#define VICO_FMI_SYSTEM_HPP

#include "vico/system.hpp"
#include "algorithm.hpp"

#include <fmilibcpp/slave.hpp>

namespace vico
{

class fmi_system : public system
{

public:
    explicit fmi_system(std::unique_ptr<algorithm> algorithm);

private:
    void add_slave(const std::string& instanceName, std::unique_ptr<fmilibcpp::slave> slave);

    void init() override;

    void step(double currentTime, double stepSize) override;

    void terminate() override;

    ~fmi_system();

private:
    std::unique_ptr<algorithm> algorithm_;
    std::vector<std::unique_ptr<fmilibcpp::slave>> slaves_;
};

} // namespace vico

#endif // VICO_FMI_SYSTEM_HPP
