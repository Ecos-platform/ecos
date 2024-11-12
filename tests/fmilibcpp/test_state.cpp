
#include "fmilibcpp/fmu.hpp"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <ecos/model_instance.hpp>

namespace
{

void test(fmilibcpp::fmu& fmu)
{

    const auto md = fmu.get_model_description();
    REQUIRE(md.canGetAndSetState);

    auto instance = fmu.new_instance("instance");
    REQUIRE(instance->enter_initialization_mode());
    REQUIRE(instance->exit_initialization_mode());

    double t = 0.0;
    double tEnd = 1.0;
    double dt = 0.1;

    void* state = nullptr;

    double tState;
    double heightAtStateChange;

    while (t <= tEnd) {

        REQUIRE(instance->step(t, dt));
        t += dt;

        if (!state && t > 0.5) {
            tState = t;
            state = instance->get_state();
            heightAtStateChange = instance->get_real(md.get_by_name("h")->vr);
        }
    }

    REQUIRE(instance->get_real(md.get_by_name("h")->vr) != Catch::Approx(heightAtStateChange));

    t = tState;
    REQUIRE(instance->set_state(state));
    REQUIRE(instance->free_state(state));

    REQUIRE(instance->get_real(md.get_by_name("h")->vr) == Catch::Approx(heightAtStateChange));

    while (t <= tEnd) {

        REQUIRE(instance->step(t, dt));

        t += dt;
    }

    REQUIRE(instance->terminate());
    instance->freeInstance();
}

} // namespace

TEST_CASE("fmi_test_state")
{
    std::string fmuPath = std::string(DATA_FOLDER) + "/fmus/3.0/ref/BouncingBall.fmu";
    auto fmu = fmilibcpp::loadFmu(fmuPath);
    test(*fmu);
}
