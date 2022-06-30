#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vico/ssp/ssp_parser.hpp>

using namespace vico;

TEST_CASE("test_ssp_parser")
{

    const auto quarter_truck = "../data/ssp/quarter-truck.ssp";
    SystemStructureDescription desc = parse_ssp(quarter_truck);

    CHECK(desc.name == "QuarterTruck");
    CHECK(desc.system.name == "QuarterTruckSystem");

}
