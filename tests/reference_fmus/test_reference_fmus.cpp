
#include <catch2/catch_test_macros.hpp>

#include "ecos/model_resolver.hpp"
#include "ecos/simulation.hpp"
#include <ecos/algorithm/fixed_step_algorithm.hpp>
#include <ecos/structure/simulation_structure.hpp>

#include <iostream>

using namespace ecos;

std::vector<std::filesystem::path> collectFMus(const std::filesystem::path& basePath)
{
    std::vector<std::filesystem::path> paths;
    for (const auto& entry : std::filesystem::directory_iterator(basePath)) {
        if (entry.is_regular_file()) {
            if (entry.path().extension() == ".fmu") {
                paths.emplace_back(entry.path());
            }
        }
    }
    return paths;
}

std::vector<std::filesystem::path> v1FMus()
{
    return collectFMus(std::string(REF_FMU_FOLDER) + "1.0/cs");
}

std::vector<std::filesystem::path> v2FMus()
{
    return collectFMus(std::string(REF_FMU_FOLDER) + "2.0");
}

std::vector<std::filesystem::path> v3FMus()
{
    return collectFMus(std::string(REF_FMU_FOLDER) + "3.0");
}

void run(const std::vector<std::filesystem::path>& paths)
{
    const auto resolver = default_model_resolver();
    for (const auto& path : paths) {
        auto fmuModel = resolver->resolve(path.string());
        REQUIRE(fmuModel);

        simulation sim(std::make_unique<fixed_step_algorithm>(1.0 / 100));
        auto slave = fmuModel->instantiate(path.stem().string());
        REQUIRE(slave);
        sim.add_slave(std::move(slave));

        sim.init();
        sim.step_for(5);
        sim.terminate();
    }
}

TEST_CASE("Run reference fmus")
{

    run(v1FMus());
    run(v2FMus());
    run(v3FMus());
}
