#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vico/ssp/ssp_parser.hpp>

using namespace vico;

TEST_CASE("test_ssp_parser")
{

    const auto quarter_truck = "../data/ssp/quarter-truck.ssp";
    SystemStructureDescription desc = parse_ssp(quarter_truck);

    CHECK(desc.name == "QuarterTruck");

    const auto system = desc.system;
    CHECK(system.name == "QuarterTruckSystem");

    const auto components = system.elements.components;
    CHECK(components.size() == 3);

    auto chassis_iter = std::find_if(components.begin(), components.end(), [](const Component& c){return c.name == "chassis";});
    REQUIRE(chassis_iter != std::end(components));
    const Component& chassis = *chassis_iter;
    CHECK(chassis.connectors.size() == 2);

}
