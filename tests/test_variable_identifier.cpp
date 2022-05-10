#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vico/structure/variable_identifier.hpp>

using namespace vico;

TEST_CASE("test_variable_identifier")
{
    variable_identifier v("instanceName.nested.variableName");

    CHECK("instanceName" == v.instanceName);
    CHECK("nested.variableName" == v.variableName);
}
