#include <catch2/catch_test_macros.hpp>

#include <util/temp_dir.hpp>
#include <util/unzipper.hpp>

using namespace ecos;

TEST_CASE("test_unzipper")
{

    const std::string quarter_truck = std::string(DATA_FOLDER) + "/ssp/quarter_truck/quarter-truck.ssp";

    std::filesystem::path sspPath;

    {
        temp_dir tmp("ssp");
        sspPath = tmp.path();
        REQUIRE(unzip(quarter_truck, tmp.path()));

        CHECK(std::filesystem::exists(sspPath));

        CHECK(std::filesystem::exists(tmp.path() / "SystemStructure.ssd"));
        CHECK(std::filesystem::exists(tmp.path() / "resources/chassis.fmu"));
        CHECK(std::filesystem::exists(tmp.path() / "resources/ground.fmu"));
        CHECK(std::filesystem::exists(tmp.path() / "resources/wheel.fmu"));
    }

    CHECK(!std::filesystem::exists(sspPath));
}
