#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <ecos/ecos.h>


TEST_CASE("Test C lib")
{

    std::string fmuPath = std::string(DATA_FOLDER) + "/fmus/2.0/20sim/ControlledTemperature.fmu";

    auto ss = ecos_simulation_structure_create_from_ssp(fmuPath.c_str());
    auto sim = ecos_simulation_create(ss, 1.0/100);

    ecos_simulation_step(sim);

    ecos_simulation_structure_destroy(ss);



}
