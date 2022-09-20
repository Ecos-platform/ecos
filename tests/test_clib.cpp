#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <ecos/ecos.h>


TEST_CASE("Test C lib")
{

    std::string fmuPath = std::string(DATA_FOLDER) + "/ssp/quarter_truck";

    auto ss = ecos_simulation_structure_create_from_ssp(fmuPath.c_str());
    if (!ss) {
        std::cerr << ecos_last_error_msg() << std::endl;
        return;
    }
    auto sim = ecos_simulation_create(ss, 1.0/100);
    if (!sim) {
        std::cerr << ecos_last_error_msg() << std::endl;
        return;
    }

    ecos_simulation_init(sim, 0, "initialValues");
    ecos_simulation_step(sim);

    ecos_simulation_structure_destroy(ss);
    ecos_simulation_destroy(sim);

}
