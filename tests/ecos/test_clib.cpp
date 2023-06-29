#include <catch2/catch_test_macros.hpp>

#include <ecos/ecos.h>

#include <iostream>

TEST_CASE("Test C lib")
{

    auto v = ecos_library_version();
    std::cout << "Using libecos version: " << v.major << "." << v.minor << "." << v.patch << std::endl;

    std::string sspPath = std::string(DATA_FOLDER) + "/ssp/quarter_truck";

    auto sim = ecos_simulation_create(sspPath.c_str(), 1.0 / 100);
    REQUIRE(sim);

    auto writer = ecos_csv_writer_create("results/quarter_truck.csv");
    ecos_simulation_add_listener(sim, "csv_writer", writer);

    ecos_simulation_init(sim, 0, "initialValues");
    ecos_simulation_step(sim, 100);

    ecos_simulation_terminate(sim);

    ecos_simulation_destroy(sim);
}
