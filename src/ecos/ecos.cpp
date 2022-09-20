
#include "ecos/ecos.h"

#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/lib_info.hpp"
#include "ecos/listeners/csv_writer.hpp"
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

struct ecos_simulation_listener
{
    std::unique_ptr<ecos::simulation_listener> cpp_listener;
};

void handle_current_exception()
{
    try {
        throw;
    } catch (const std::exception& ex) {
        g_last_error_msg = ex.what();
    }
}


ecos_simulation_t* ecos_simulation_create(const char* sspPath, double stepSize)
{
    try {
        auto ss = ecos::load_ssp(sspPath);

        auto algorithm = std::make_unique<ecos::fixed_step_algorithm>(stepSize);
        auto sim = std::make_unique<ecos_simulation_t>();
        sim->cpp_sim = ss->load(std::move(algorithm));
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

bool ecos_simulation_add_csv_writer(ecos_simulation_t* sim, const char* resultFile, const char* configFile)
{
    try {

        auto writer = std::make_unique<ecos::csv_writer>(resultFile);
        if (configFile) {
            writer->config().load(configFile);
        }

        sim->cpp_sim->add_listener(std::move(writer));

        return true;
    } catch (...) {
        handle_current_exception();
        return false;
    }
}

void ecos_simulation_terminate(ecos_simulation_t* sim)
{
    if (!sim) return;
    sim->cpp_sim->terminate();
}

void ecos_simulation_destroy(ecos_simulation_t* sim)
{
    if (!sim) return;
    delete (sim);
}


ecos_version* ecos_library_version()
{
    ecos::version v = ecos::library_version();
    return new ecos_version {v.major, v.minor, v.patch};
}
