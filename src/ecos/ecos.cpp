
#include "ecos/ecos.h"

#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/simulation.hpp"
#include "ecos/ssp/ssp_loader.hpp"

#include <memory>

std::string g_last_error_msg;

const char* ecos_last_error_msg()
{
    return g_last_error_msg.c_str();
}

struct ecos_simulation
{
    std::unique_ptr<ecos::simulation> cpp_sim;
};

struct ecos_simulation_structure
{
    std::unique_ptr<ecos::simulation_structure> cpp_structure;
};

void handle_current_exception()
{
    try {
        throw;
    } catch (const std::exception& ex) {
        g_last_error_msg = ex.what();
    }
}


ecos_simulation_structure_t* ecos_simulation_structure_create_from_ssp(const char* path)
{
    try {
        auto ss = ecos::load_ssp(path);
        auto ss_c = std::make_unique<ecos_simulation_structure_t>();
        ss_c->cpp_structure = std::move(ss);

        return ss_c.release();
    } catch (...) {
        handle_current_exception();
        return nullptr;
    }
}

ecos_simulation_t* ecos_simulation_create(ecos_simulation_structure_t* ss, double stepSize)
{
    try {
        auto algorithm = std::make_unique<ecos::fixed_step_algorithm>(stepSize);
        auto sim = std::make_unique<ecos_simulation_t>();
        sim->cpp_sim = ss->cpp_structure->load(std::move(algorithm));
        return sim.release();
    } catch (...) {
        handle_current_exception();
        return nullptr;
    }
}

bool ecos_simulation_init(ecos_simulation_t* sim, double startTime, const char* parameterSet)
{
    try {
        if (!parameterSet) {
            sim->cpp_sim->init(startTime);
        } else {
            sim->cpp_sim->init(startTime, parameterSet);
        }
        return true;
    } catch (...) {
        handle_current_exception();
        return false;
    }
}

void ecos_simulation_step(ecos_simulation_t* sim, size_t numSteps)
{
    sim->cpp_sim->step(numSteps);
}

void ecos_simulation_structure_destroy(ecos_simulation_structure_t* ss)
{
    if (!ss) return;
    delete (ss);
}

void ecos_simulation_destroy(ecos_simulation_t* sim)
{
    if (!sim) return;
    delete (sim);
}
