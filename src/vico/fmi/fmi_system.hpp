
#ifndef VICO_FMI_SYSTEM_HPP
#define VICO_FMI_SYSTEM_HPP

#include "algorithm.hpp"

#include "vico/system.hpp"

#include <fmilibcpp/slave.hpp>

namespace vico
{

class fmi_system : public system
{

public:
    explicit fmi_system(std::unique_ptr<algorithm> algorithm);

    void add_slave(std::unique_ptr<fmilibcpp::slave> slave);

    void init(double startTime) override;

    void step(double currentTime, double stepSize) override;

    void terminate() override;

    ~fmi_system() override;

private:
    std::unique_ptr<algorithm> algorithm_;
    std::unordered_map<std::string, std::unique_ptr<fmilibcpp::slave>> slaves_;
};

} // namespace vico

#endif // VICO_FMI_SYSTEM_HPP
