#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <ecos/util/unzipper.hpp>
#include <ecos/util/temp_dir.hpp>

using namespace ecos;

TEST_CASE("test_unzipper")
{

    const std::string quarter_truck = std::string(DATA_FOLDER) + "/ssp/quarter_truck/quarter-truck.ssp";

    temp_dir tmp("ssp");
    bool success = unzip(quarter_truck, tmp.path());
    REQUIRE(success);
    REQUIRE(std::filesystem::exists(tmp.path() / "SystemStructure.ssd"));
    REQUIRE(std::filesystem::exists(tmp.path() / "resources/chassis.fmu"));
    REQUIRE(std::filesystem::exists(tmp.path() / "resources/ground.fmu"));
    REQUIRE(std::filesystem::exists(tmp.path() / "resources/wheel.fmu"));

}
