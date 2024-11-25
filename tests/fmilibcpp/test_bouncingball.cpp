
#include "fmilibcpp/fmu.hpp"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace
{

void test(fmilibcpp::fmu& fmu)
{
    const auto d = fmu.get_model_description();
    CHECK(d.modelName == "BouncingBall");
    CHECK(d.description ==
        "This model calculates the trajectory, over time, of a ball dropped from a height of 1 m");

    auto slave = fmu.new_instance("instance");
    REQUIRE(slave);
    REQUIRE(slave->enter_initialization_mode());
    REQUIRE(slave->exit_initialization_mode());

    std::vector<fmilibcpp::value_ref> vr{0};

    std::vector<double> realRef(1);

    double t = 0.0;
    double tEnd = 1.0;
    double dt = 0.1;

    double h;
    while (t <= tEnd) {


        REQUIRE(slave->step(t, dt));

        h = slave->get_real(1);

        t += dt;
    }

    CHECK(h == Catch::Approx(0.0235492));

    REQUIRE(slave->terminate());
    slave->freeInstance();
}

} // namespace

TEST_CASE("fmi_test_bouncingball")
{
    std::string fmuPath = std::string(DATA_FOLDER) + "/fmus/3.0/ref/BouncingBall.fmu";
    const auto fmu = fmilibcpp::loadFmu(fmuPath);
    test(*fmu);
}
