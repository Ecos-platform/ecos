
#include "smd_common.hpp"

int main()
{

    set_logging_level(log::level::debug);

    const std::filesystem::path fmuDir = std::string(DATA_FOLDER) + "/fmus/1.0/mass_spring_damper";

    simulation_structure ss;
    ss.add_model("mass", (fmuDir / "Mass.fmu").string());
    ss.add_model("spring", (fmuDir / "Spring.fmu").string());
    ss.add_model("damper", (fmuDir / "Damper.fmu").string());

    addConnections(ss);
    addParameterSets(ss);

    run(ss, 10);
}
