#include "ct_common.hpp"

int main()
{
    const std::string fmuPath{std::string(DATA_FOLDER) +"/fmus/2.0/20sim/ControlledTemperature.fmu"};
    return run(fmuPath, true);
}
