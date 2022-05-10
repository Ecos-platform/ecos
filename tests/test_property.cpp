#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vico/property.hpp>

using namespace vico;

TEST_CASE("test_property")
{

    {
        int value = -100;
        property_t<int> p(
            [&] { return value; },
            [&](auto v) { value = v; });

        CHECK(value == p.get_value());
        p(value - 1);
        CHECK(p.get_value() == -101);
        CHECK(value == -101);
    }

    {
        double value = -100;
        property_t<double> p1(
            [&] { return value; },
            [&](auto v) { value = v; });

        CHECK(value == Approx(p1.get_value()));
        p1(value - 1);
        CHECK(p1.get_value() == Approx(-101));
        CHECK(value == Approx(-101));
    }
}
