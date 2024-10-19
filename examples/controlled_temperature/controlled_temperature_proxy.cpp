#include "ct_common.hpp"

int main()
{
    const std::string fmuPath{"proxyfmu://127.0.0.1?file=" + std::string(DATA_FOLDER) +"/fmus/2.0/20sim/ControlledTemperature.fmu"};
    run(fmuPath);
}
