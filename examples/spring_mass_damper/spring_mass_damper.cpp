
#include "smd_common.hpp"

int main()
{

    log::create_default_logger(spdlog::level::debug);

    simulation_structure ss;
    ss.add_model("mass", std::string(DATA_FOLDER) + "/fmus/1.0/Mass.fmu");
    ss.add_model("spring", std::string(DATA_FOLDER) + "/fmus/1.0/Spring.fmu");
    ss.add_model("damper", std::string(DATA_FOLDER) + "/fmus/1.0/Damper.fmu");

    addConnections(ss);
    addParmeterSets(ss);

    run(ss);
}
