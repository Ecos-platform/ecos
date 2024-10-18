#include <catch2/catch_test_macros.hpp>

#include <ecos/ecos.h>

#include <fstream>
#include <iostream>

TEST_CASE("Test C lib")
{

    auto v = ecos_library_version();
    std::cout << "Using libecos version: " << v.major << "." << v.minor << "." << v.patch << std::endl;

    std::string sspPath = std::string(DATA_FOLDER) + "/ssp/quarter_truck";

    auto sim = ecos_simulation_create(sspPath.c_str(), 1.0 / 100);
    REQUIRE(sim);

    std::string csvFile("results/quarter_truck.csv");
    auto writer = ecos_csv_writer_create(csvFile.c_str());
    ecos_simulation_add_listener(sim, "csv_writer", writer);

    ecos_simulation_init(sim, 0, "initialValues");

    double value;
    CHECK(ecos_simulation_get_real(sim, "chassis::p.f", &value));
    CHECK(value == 0);
    ecos_simulation_step(sim, 100);

    CHECK(ecos_simulation_get_real(sim, "chassis::p.f", &value));
    CHECK(value > 0);

    ecos_simulation_terminate(sim);

    ecos_simulation_destroy(sim);

    std::ifstream f(csvFile);
    CHECK(f.is_open());

    std::string line;
    std::vector<std::string> data;
    while (std::getline(f, line)) {
        data.emplace_back(line);
    }

    REQUIRE(data.size() == 102);
    CHECK(data[1] == "0, 0, 0.000000, 0.000000, 400.000000, 1.000000, 3922.660000, 0.000000, 1.000000, 15000.000000, 0.000000, 400.000000, 400.000000, 3922.660000, 0.000000, 0.000000, -3922.660000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 3922.660000, 3922.660000, 0.000000, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, -3922.660000, 0.000000, 1000.000000, 0.000000, 3922.660000, 3922.660000, 3922.660000, 3922.660000, 0.000000, 0.000000, 0.261511, 0.000000, 3922.660000, 0.000000, 0.000000, 0.261511, 0.000000, 1.000000, 3922.660000, 0.000000, 400.000000, 4314.926000, 40.000000, 0.000000, 0.500000, 0.000000, 150000.000000, 40.000000, 0.000000, 0.000000, 0.000000, -392.266000, 0.000000, 4314.926000, 4314.926000, 0.000000, 0.000000, 0.000000, -392.266000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 0.000000, 4314.926000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 4314.926000, 0.000000, 0.000000, 40.000000, 0.000000, 0.000000, 4314.926000, 4314.926000, 4314.926000, 4314.926000, 0.000000, 3922.660000, 0.000000, 0.000000, 0.028766, 0.028766, 3922.660000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 10.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.100000, 1.000000, 4314.926000, 0.000000, 0.000000");
    CHECK(data[101] == "100, 1, 0.000087, -0.205829, 400.000000, 0.999944, 3922.326439, 0.000087, 0.999944, 15000.000000, -0.205829, 400.000000, 400.000000, 3922.454171, 0.034601, 0.000087, -3922.660000, -0.205829, 0.000087, 0.000087, 0.000087, 0.000128, 3922.454171, 3922.326439, 0.127732, 1.000000, 0.000128, 0.000128, 0.000128, -0.205829, 0.000087, 0.000087, 0.127732, -3922.660000, 0.000128, 1000.000000, 0.000087, 3922.454171, 3922.454171, 3922.454171, 3922.454171, 0.000128, 0.000214, 0.261511, 0.000087, 3922.454171, 0.000161, 0.000128, 0.261488, 0.000000, 0.999944, -0.354311, 0.000161, 400.000000, 4314.276786, 40.000000, 3922.365097, 0.500004, -0.000161, 150000.000000, 40.000000, -0.000161, 0.000161, 0.000161, -392.266000, -0.000000, 4314.276786, 4314.276786, 0.000161, 0.000000, 0.000161, -392.266000, 0.000161, 0.500004, -0.000161, -0.000161, 0.000161, -0.000161, 4314.276786, -0.000000, -0.000161, 3922.365097, 0.000000, 3922.365097, 4314.276786, 0.000161, 0.000000, 40.000000, 0.000161, 0.000161, 4314.276786, 4314.276786, 4314.276786, 4314.276786, 6.217388, 3922.454171, 0.000161, -0.000161, 0.028762, 0.028766, -0.354311, 0.006443, 0.000161, 0.500000, 0.500004, 0.000000, 4314.567961, -0.065797, 0.100000, 4314.567961, 6.217388, 0.100000, 6.217388, 10.000000, 6.217388, 0.100000, 0.100000, 0.100000, 0.100000, 1.000000, 4314.276786, 6.217388, 0.100000");
}
