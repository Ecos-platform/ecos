
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/model_resolver.hpp"
#include "ecos/simulation_runner.hpp"
#include "ecos/logger/logger.hpp"

using namespace ecos;

TEST_CASE("test simulation runner")
{

    set_logging_level(ecos::log::level::debug);

    std::string fmuPath = std::string(DATA_FOLDER) + "/fmus/2.0/20sim/ControlledTemperature.fmu";
    auto resolver = default_model_resolver();
    auto fmuModel = resolver->resolve(fmuPath);

    simulation sim(std::make_unique<fixed_step_algorithm>(1.0 / 100));
    sim.add_slave(fmuModel->instantiate("slave"));
    sim.init();

    auto runner = simulation_runner(sim);
    runner.set_real_time_factor(1);
    auto future = runner.run_while([&sim] {
        return sim.time() < 1;
    });

    future.get();

    log::debug("Simulated {:.3f}s in {:.4f}s, RTF={:.3f}", sim.time(), runner.wall_clock(), runner.real_time_factor());

    CHECK_THAT(runner.real_time_factor(), Catch::Matchers::WithinAbs(runner.target_real_time_factor(), 0.1));

    sim.terminate();
}