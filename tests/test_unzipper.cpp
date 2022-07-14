#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <ecos/util/unzipper.hpp>
#include <ecos/util/temp_dir.hpp>

using namespace ecos;

TEST_CASE("test_unzipper")
{

    const auto quarter_truck = "../data/ssp/quarter_truck/quarter-truck.ssp";

    temp_dir tmp("ssp");
    auto folder = unzip(quarter_truck, tmp.path());
    REQUIRE(folder);
    REQUIRE(std::filesystem::exists(tmp.path() / "SystemStructure.ssd"));
    REQUIRE(std::filesystem::exists(tmp.path() / "resources/chassis.fmu"));
    REQUIRE(std::filesystem::exists(tmp.path() / "resources/ground.fmu"));
    REQUIRE(std::filesystem::exists(tmp.path() / "resources/wheel.fmu"));

}
