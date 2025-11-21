#include <catch2/catch_test_macros.hpp>

#include <ecos/variable_identifier.hpp>

#include <sstream>

using namespace ecos;

TEST_CASE("test_variable_identifier")
{

    SECTION("Ctor")
    {
        variable_identifier v1("instanceName::nested.variableName");

        CHECK("instanceName" == v1.instance_name());
        CHECK("nested.variableName" == v1.variable_name());
    }

    SECTION("Nested variableName")
    {
        variable_identifier v2("instanceName", "nested.variableName");

        CHECK("instanceName" == v2.instance_name());
        CHECK("nested.variableName" == v2.variable_name());
    }

    SECTION("Comparison test")
    {
        variable_identifier a("instance1", "variable1");
        variable_identifier b("instance2", "variable2");
        variable_identifier c("instance1", "variable1");

        REQUIRE(a == c);
        CHECK_FALSE(a == b);
        CHECK(a < b);
        CHECK_FALSE(b < a);
    }

    SECTION("str()")
    {
        variable_identifier a("instance1", "variable1");

        REQUIRE(a.str() == "instance1::variable1");
    }

    SECTION("Output stream overload")
    {
        variable_identifier a("car", "speed");
        std::ostringstream oss;
        oss << a;

        REQUIRE(oss.str() == "car::speed");
    }

    SECTION("Pattern matching")
    {
        variable_identifier a("train", "mass");
        // Exact match with full wildcard
        CHECK(a.matches("*::*"));
        CHECK(a.matches("train::*"));
        CHECK(a.matches("*::mass"));
        CHECK(a.matches("train::mass"));

        // Prefix/suffix wildcard
        CHECK(a.matches("t*::m*")); // both fields prefix wildcard
        CHECK(a.matches("t*in::ma*s")); // inner wildcard
        CHECK(a.matches("*ain::*ass")); // suffix match
        CHECK(a.matches("tr*in::m*ss")); // partial inner matches

        // Negative tests
        CHECK(!a.matches("dog::*")); // wrong instance
        CHECK(!a.matches("*::weight")); // wrong variable
        CHECK(!a.matches("train::")); // incomplete pattern
        CHECK(!a.matches("::mass")); // missing instanceName
        CHECK(!a.matches("::train")); // no instanceName
        CHECK(!a.matches("train::")); // no variableName
        CHECK(!a.matches("::")); // no instanceName or variableName

        // Full wildcard at start, middle, end
        CHECK(a.matches("*rain::mass")); // instanceName ends with 'rain'
        CHECK(a.matches("tr*in::mass")); // instanceName contains 'r'
        CHECK(a.matches("train::ma*")); // variableName starts with 'ma'
        CHECK(a.matches("train::m*ss")); // variableName has one character replaced

        // Edge: wildcard only field
        CHECK(a.matches("*::m*")); // any instance, variable starts with m
        CHECK(a.matches("t*::*")); // instance starts with t
        CHECK(a.matches("*::m*ss")); // ends with 'ss'

        // Totally unrelated
        CHECK(!a.matches("car::speed"));
        CHECK(!a.matches("bus::length"));
    }
}
