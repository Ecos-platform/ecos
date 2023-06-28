#include "common.hpp"

int main()
{
    const std::string fmuPath{"proxyfmu://localhost?file=" + std::string(DATA_FOLDER) +"/fmus/2.0/20sim/ControlledTemperature.fmu"};
    run(fmuPath);
}
