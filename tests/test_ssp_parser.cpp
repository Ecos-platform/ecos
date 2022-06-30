#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vico/ssp/ssp_parser.hpp>

using namespace vico;

namespace
{

void checkSystemStructure(const SystemStructureDescription& desc)
{
    CHECK(desc.name == "QuarterTruck");

    const auto system = desc.system;
    CHECK(system.name == "QuarterTruckSystem");

    const auto components = system.elements.components;
    CHECK(components.size() == 3);

    REQUIRE(components.count("chassis"));
    REQUIRE(components.count("wheel"));
    REQUIRE(components.count("ground"));

    const Component& chassis = components.at("chassis");
    CHECK(chassis.connectors.size() == 2);

    const Component& wheel = components.at("wheel");
    CHECK(wheel.connectors.size() == 4);

    const Component& ground = components.at("ground");
    CHECK(ground.connectors.size() == 2);
}

} // namespace

TEST_CASE("test_ssp_parser_archive")
{
    const auto quarter_truck = "../data/ssp/quarter_truck/quarter-truck.ssp";
    SystemStructureDescription desc = parse_ssp(quarter_truck);
    checkSystemStructure(desc);
}

TEST_CASE("test_ssp_parser_folder")
{
    const auto quarter_truck = "../data/ssp/quarter_truck";
    SystemStructureDescription desc = parse_ssp(quarter_truck);
    checkSystemStructure(desc);
}
