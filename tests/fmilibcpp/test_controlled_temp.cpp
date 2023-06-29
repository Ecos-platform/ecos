#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "ecos/fmi/fmu.hpp"

using namespace ecos::fmi;

namespace
{

void test(fmu& fmu)
{
    const auto d = fmu.get_model_description();
    CHECK(d.modelName == "ControlledTemperature");
    CHECK(d.modelIdentifier == "ControlledTemperature");
    CHECK(d.guid == "{06c2700b-b39c-4895-9151-304ddde28443}");
    CHECK(d.generationTool == "20-sim");

    auto slave = fmu.new_instance("instance");
    REQUIRE(slave->setup_experiment());
    REQUIRE(slave->enter_initialization_mode());
    REQUIRE(slave->exit_initialization_mode());

    std::vector<value_ref> vr{47};
    std::vector<double> realRef(1);

    slave->get_real(vr, realRef);
    CHECK_THAT(realRef[0], Catch::Matchers::WithinRel(298.));

    REQUIRE(slave->step(0.0, 0.1));

    slave->get_real(vr, realRef);
    CHECK(realRef[0] < 298);

    REQUIRE(slave->terminate());
}

} // namespace

TEST_CASE("fmi_test_controlled_temp")
{
    std::string fmuPath = std::string(DATA_FOLDER) + "/fmus/2.0/20sim/ControlledTemperature.fmu";
    auto fmu = loadFmu(fmuPath);
    test(*fmu);
}
