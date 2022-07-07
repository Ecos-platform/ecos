#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vico/variable_identifier.hpp>

using namespace vico;

TEST_CASE("test_variable_identifier")
{
    variable_identifier v1("instanceName::nested.variableName");

    CHECK("instanceName" == v1.instanceName);
    CHECK("nested.variableName" == v1.variableName);

    variable_identifier v2("instanceName", "nested.variableName");

    CHECK("instanceName" == v2.instanceName);
    CHECK("nested.variableName" == v2.variableName);
}
