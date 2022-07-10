#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vico/property.hpp>

using namespace vico;

TEST_CASE("test_property")
{
    {
        int value = -100;
        property_t<int> p(
            {"::intValue"},
            [&] { return value; },
            [&](auto v) { value = v; });

        CHECK(value == p.get_value());
        p.set_value(value - 1);
        p.applySet();
        CHECK(p.get_value() == -101);
        CHECK(value == -101);
        REQUIRE(p.id.instanceName.empty());
        REQUIRE(p.id.variableName == "intValue");
    }

    {
        double value = -100;
        property_t<double> p(
            {"::doubleValue"},
            [&] { return value; },
            [&](auto v) { value = v; });

        CHECK(value == Approx(p.get_value()));
        p.set_value(value - 1);
        p.applySet();
        CHECK(p.get_value() == Approx(-101));
        CHECK(value == Approx(-101));
    }
}
