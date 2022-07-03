#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vico/util/unzipper.hpp>

using namespace vico;

TEST_CASE("test_unzipper")
{

    const auto quarter_truck = "../data/ssp/quarter_truck/quarter-truck.ssp";

    auto folder = unzip(quarter_truck);
    REQUIRE(folder);
    REQUIRE(fs::exists(folder->path() / "SystemStructure.ssd"));
    REQUIRE(fs::exists(folder->path() / "resources/chassis.fmu"));
    REQUIRE(fs::exists(folder->path() / "resources/ground.fmu"));
    REQUIRE(fs::exists(folder->path() / "resources/wheel.fmu"));

}
