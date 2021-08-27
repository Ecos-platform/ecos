
#ifndef VICO_FMI_SYSTEM_HPP
#define VICO_FMI_SYSTEM_HPP

#include "vico/system.hpp"

namespace vico {

class fmi_system: public system {

    void init() override
    {
    }

    void step(double currentTime, double stepSize) override
    {
    }

    void terminate() override
    {
    }

private:

};

}

#endif // VICO_FMI_SYSTEM_HPP
