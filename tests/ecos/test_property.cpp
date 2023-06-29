#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <ecos/property.hpp>

using namespace ecos;

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
        CHECK(p.id.instanceName.empty());
        CHECK(p.id.variableName == "intValue");
    }

    {
        double value = -100;
        property_t<double> p(
            {"::doubleValue"},
            [&] { return value; },
            [&](auto v) { value = v; });

        CHECK_THAT(value,Catch::Matchers::WithinRel(p.get_value()));
        p.set_value(value - 1);
        p.applySet();
        CHECK_THAT(p.get_value(),Catch::Matchers::WithinRel(-101.));
        CHECK_THAT(value,Catch::Matchers::WithinRel(-101.));
    }
}
