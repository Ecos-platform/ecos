#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <ecos/ecos.h>


TEST_CASE("Test C lib")
{

    std::string sspPath = std::string(DATA_FOLDER) + "/ssp/quarter_truck";

    auto sim = ecos_simulation_create(sspPath.c_str(), 1.0 / 100);
    REQUIRE(sim);

    REQUIRE(ecos_simulation_add_csv_writer(sim, "results.csv"));

    ecos_simulation_init(sim, 0, "initialValues");
    ecos_simulation_step(sim, 100);

    ecos_simulation_destroy(sim);
}
