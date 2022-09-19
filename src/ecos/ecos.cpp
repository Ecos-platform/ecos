
#include "ecos/ecos.h"

#include "ecos/simulation.hpp"
#include "ecos/ssp/ssp_loader.hpp"
#include "ecos/algorithm/fixed_step_algorithm.hpp"

#include <memory>

std::string g_last_error_msg;

const char* last_error_msg()
{
    return g_last_error_msg.c_str();
}

struct ecos_simulation_s
{
    std::unique_ptr<ecos::simulation> cpp_sim;
};

struct ecos_simulation_structure_s
{
    std::unique_ptr<ecos::simulation_structure> cpp_structure;
};

void handle_current_exception() {
    try {
        throw;
    } catch (const std::exception& ex) {
        g_last_error_msg = ex.what();
    }
}


ecos_simulation_structure_s* ecos_simulation_structure_create_from_ssp(const char* path)
{
    try {
        auto ss = ecos::load_ssp(path);
        auto ss_c = std::make_unique<ecos_simulation_structure_s>();
        ss_c->cpp_structure = std::move(ss);

        return ss_c.release();
    } catch (...) {
        return nullptr;
    }
}

ecos_simulation_s* ecos_simulation_create(ecos_simulation_structure_s* ss, double stepSize)
{
    auto algorithm = std::make_unique<ecos::fixed_step_algorithm>(stepSize);
    auto sim = std::make_unique<ecos_simulation_s>();
    sim->cpp_sim = ss->cpp_structure->load(std::move(algorithm));
    return sim.release();
}

void ecos_simulation_step(ecos_simulation_s* sim, size_t numSteps)
{
    sim->cpp_sim->step(numSteps);
}

void ecos_simulation_structure_destroy(ecos_simulation_structure_s* ss)
{
    if (!ss) return;
    delete(ss);
}
