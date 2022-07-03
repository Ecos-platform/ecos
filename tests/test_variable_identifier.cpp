#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "vico/structure/variable_identifier.hpp"
#include <utility>

using namespace vico;

TEST_CASE("test_variable_identifier")
{
    variable_identifier v1("instanceName.nested.variableName");

    CHECK("instanceName" == v1.instanceName);
    CHECK("nested.variableName" == v1.variableName);

    variable_identifier v2(std::make_pair("instanceName", "nested.variableName"));

    CHECK("instanceName" == v2.instanceName);
    CHECK("nested.variableName" == v2.variableName);
}
