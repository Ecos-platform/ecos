
#include "catch2/matchers/catch_matchers_vector.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <utility>

#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/scenario.hpp"
#include "ecos/simulation.hpp"

#include <iostream>

using namespace ecos;

namespace
{
class mock_instance : public model_instance
{
public:
    explicit mock_instance(const std::string& name)
        : model_instance(name)
    {
        properties_.add_int_property(integer_prop_);
        properties_.add_real_property(real_prop_);
        properties_.add_bool_property(boolean_prop_);
        properties_.add_string_property(string_prop_);
    }

    void set_debug_logging(bool flag) override { }
    void enter_initialization_mode(double start) override { }
    void exit_initialization_mode() override { }
    void step(double currentTime, double stepSize) override { }
    void terminate() override { }
    void reset() override { }

private:
    int integer_{};
    double real_{};
    bool boolean_{};
    std::string string_{};

    property_t<int> integer_prop_ = property_t<int>(
        {instanceName_, "integer_prop"},
        [this] { return integer_; },
        [this](auto v) { integer_ = v; });

    property_t<double> real_prop_ = property_t<double>(
        {instanceName_, "real_prop"},
        [this] { return real_; },
        [this](auto v) { real_ = v; });

    property_t<bool> boolean_prop_ = property_t<bool>(
        {instanceName_, "boolean_prop"},
        [this] { return boolean_; },
        [this](auto v) { boolean_ = v; });

    property_t<std::string> string_prop_ = property_t<std::string>(
        {instanceName_, "string_prop"},
        [this] { return string_; },
        [this](auto v) { string_ = std::move(v); });
};

} // namespace

TEST_CASE("Test Scenario")
{

    const auto dt = 1.0 / 100;
    auto algorithm = std::make_unique<fixed_step_algorithm>(dt);
    simulation sim(std::move(algorithm));
    sim.add_slave(std::make_unique<mock_instance>("mock"));

    auto sc = std::make_unique<scenario>();
    sc->add_action(2, "mock::integer_prop", 1);
    sc->add_action(2, "mock::real_prop", 10.0);
    sc->add_action(2, "mock::boolean_prop", true);
    sc->add_action(2, "mock::string_prop", "Hello");

    sim.add_listener("scenario", std::move(sc));

    auto integer_prop = sim.get_int_property("mock::integer_prop");
    REQUIRE(integer_prop);

    auto real_prop = sim.get_real_property("mock::real_prop");
    REQUIRE(real_prop);

    REQUIRE(integer_prop->get_value() == 0);
    REQUIRE(real_prop->get_value() == Catch::Approx(0.));


    sim.init();
    sim.step(500); // step to time 5.0
    sim.step(1); // scenarios run at the beginning of the step

    CHECK(integer_prop->get_value() == 1);
    CHECK(real_prop->get_value() == Catch::Approx(10.));
}