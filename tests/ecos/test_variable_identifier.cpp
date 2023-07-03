#include <catch2/catch_test_macros.hpp>

#include <ecos/variable_identifier.hpp>

#include <sstream>

using namespace ecos;

TEST_CASE("test_variable_identifier")
{

    SECTION("Ctor")
    {
        variable_identifier v1("instanceName::nested.variableName");

        CHECK("instanceName" == v1.instanceName);
        CHECK("nested.variableName" == v1.variableName);
    }

    SECTION("Nested variableName")
    {
        variable_identifier v2("instanceName", "nested.variableName");

        CHECK("instanceName" == v2.instanceName);
        CHECK("nested.variableName" == v2.variableName);
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
}
