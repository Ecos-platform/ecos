#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vico/ssp/ssp_parser.hpp>

using namespace vico;

namespace
{

void checkSystemStructure(const SystemStructureDescription& ssd)
{
    CHECK(ssd.name == "QuarterTruck");

    const auto system = ssd.system;
    CHECK(system.name == "QuarterTruckSystem");

    CHECK(system.connections.size() == 4);

    const auto components = system.elements.components;
    CHECK(components.size() == 3);

    REQUIRE(components.count("chassis"));
    REQUIRE(components.count("wheel"));
    REQUIRE(components.count("ground"));

    const Component& chassis = components.at("chassis");
    CHECK(chassis.source == "resources/chassis.fmu");
    REQUIRE(chassis.connectors.size() == 2);
    CHECK(chassis.connectors[0].name == "p.e");
    CHECK(chassis.connectors[0].kind == "output");
    CHECK(!chassis.connectors[0].type.unit.has_value());
    CHECK(!chassis.connectors[0].type.isReal());
    CHECK(chassis.connectors[1].name == "p.f");
    CHECK(chassis.connectors[1].kind == "input");
    CHECK(!chassis.connectors[1].type.unit.has_value());
    CHECK(!chassis.connectors[1].type.isReal());

    const Component& wheel = components.at("wheel");
    CHECK(wheel.source == "resources/wheel.fmu");
    REQUIRE(wheel.connectors.size() == 4);
    CHECK(wheel.connectors[0].name == "p.f");
    CHECK(wheel.connectors[1].name == "p1.e");
    CHECK(wheel.connectors[2].name == "p.e");
    CHECK(wheel.connectors[3].name == "p1.f");

    const auto& wheelParameters = wheel.parameterSets;
    REQUIRE(wheelParameters.at("initialValues").parameters.size() == 3);

    const Component& ground = components.at("ground");
    CHECK(ground.source == "resources/ground.fmu");
    CHECK(ground.connectors.size() == 2);

    const auto& groundParameters = ground.parameterSets;
    REQUIRE(groundParameters.empty());

    REQUIRE(system.elements.parameterSets.size() == 1);
    REQUIRE(system.elements.parameterSets.count("initialValues"));
    const auto& initialValues = system.elements.parameterSets.at("initialValues");
    REQUIRE(initialValues.size() == 2);
    REQUIRE(initialValues.count("chassis"));
    REQUIRE(initialValues.at("chassis").size() == 3);
    REQUIRE(initialValues.count("wheel"));
    REQUIRE(initialValues.at("wheel").size() == 3);

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
