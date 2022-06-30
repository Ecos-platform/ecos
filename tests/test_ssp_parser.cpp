#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <vico/ssp/ssp_parser.hpp>

using namespace vico;

TEST_CASE("test_ssp_parser")
{

    const auto quarter_truck = "../data/ssp/quarter-truck.ssp";

    parse_ssp(quarter_truck);

}
