#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vico/util/unzipper.hpp>

using namespace vico;

TEST_CASE("test_unzipper")
{

    const auto quarter_truck = "../data/ssp/quarter-truck.ssp";

    auto folder = unzip(quarter_truck);
    REQUIRE(folder);

}
