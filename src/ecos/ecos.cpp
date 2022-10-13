
#include "ecos/ecos.h"

#include "ecos/logger.hpp"
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

void set_log_level(const char* level) {
    if (std::string("trace") == level) {
        ecos::log::set_logging_level(ecos::log::level::trace);
    } else if (std::string("debug") == level) {
        ecos::log::set_logging_level(ecos::log::level::debug);
    } else if (std::string("info") == level) {
        ecos::log::set_logging_level(ecos::log::level::info);
    } else if (std::string("warn") == level) {
        ecos::log::set_logging_level(ecos::log::level::warn);
    } else if (std::string("err") == level) {
        ecos::log::set_logging_level(ecos::log::level::err);
    } else if (std::string("off") == level) {
        ecos::log::set_logging_level(ecos::log::level::off);
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

void ecos_simulation_step_until(ecos_simulation_t* sim, double timePoint)
{
    sim->cpp_sim->step_until(timePoint);
}

bool ecos_simulation_get_integer(ecos_simulation_t* sim, const char* identifier, int* value)
{
    try {
        auto prop = sim->cpp_sim->get_int_property(identifier);
        *value = prop->get_value();
        return true;
    } catch (...) {
        handle_current_exception();
        return false;
    }
}

bool ecos_simulation_get_real(ecos_simulation_t* sim, const char* identifier, double* value)
{
    try {
        auto prop = sim->cpp_sim->get_real_property(identifier);
        *value = prop->get_value();
        return true;
    } catch (...) {
        handle_current_exception();
        return false;
    }
}

bool ecos_simulation_get_bool(ecos_simulation_t* sim, const char* identifier, bool* value)
{
    try {
        auto prop = sim->cpp_sim->get_bool_property(identifier);
        *value = prop->get_value();
        return true;
    } catch (...) {
        handle_current_exception();
        return false;
    }
}

bool ecos_simulation_get_string(ecos_simulation_t* sim, const char* identifier, const char* value)
{
    try {
        auto prop = sim->cpp_sim->get_string_property(identifier);
        value = prop->get_value().c_str();
        return true;
    } catch (...) {
        handle_current_exception();
        return false;
    }
}

bool ecos_simulation_set_integer(ecos_simulation_t* sim, const char* identifier, int value)
{
    try {
        auto prop = sim->cpp_sim->get_int_property(identifier);
        prop->set_value(value);
        return true;
    } catch (...) {
        handle_current_exception();
        return false;
    }
}

bool ecos_simulation_set_real(ecos_simulation_t* sim, const char* identifier, double value)
{
    try {
        auto prop = sim->cpp_sim->get_real_property(identifier);
        prop->set_value(value);
        return true;
    } catch (...) {
        handle_current_exception();
        return false;
    }
}

bool ecos_simulation_set_bool(ecos_simulation_t* sim, const char* identifier, bool value)
{
    try {
        auto prop = sim->cpp_sim->get_bool_property(identifier);
        prop->set_value(value);
        return true;
    } catch (...) {
        handle_current_exception();
        return false;
    }
}

bool ecos_simulation_set_string(ecos_simulation_t* sim, const char* identifier, const char* value)
{
    try {
        auto prop = sim->cpp_sim->get_string_property(identifier);
        prop->set_value(value);
        return true;
    } catch (...) {
        handle_current_exception();
        return false;
    }
}


void ecos_simulation_add_listener(ecos_simulation_t* sim, const char* name, ecos_simulation_listener_t* listener)
{
    if (listener) {
        sim->cpp_sim->add_listener(name, std::move(listener->cpp_listener));
        delete listener;
        listener = nullptr;
    }
}

void ecos_simulation_remove_listener(ecos_simulation_t* sim, const char* name)
{
    if (name) {
        sim->cpp_sim->remove_listener(name);
    }
}

ecos_simulation_listener_t* ecos_csv_writer_create(const char* resultFile, const char* logConfig, const char* plotConfig)
{
    try {

        auto writer = std::make_unique<ecos::csv_writer>(resultFile);
        if (logConfig) {
            writer->config().load(logConfig);
        }
        if (plotConfig) {
            writer->config().enable_plotting(plotConfig);
        }

        auto l = std::make_unique<ecos_simulation_listener_t>();
        l->cpp_listener = std::move(writer);

        return l.release();

    } catch (...) {
        handle_current_exception();
        return nullptr;
    }
}

void ecos_simulation_terminate(ecos_simulation_t* sim)
{
    if (sim) {
        sim->cpp_sim->terminate();
    }
}

void ecos_simulation_destroy(ecos_simulation_t* sim)
{
    delete sim;
    sim = nullptr;
}


ecos_version ecos_library_version()
{
    ecos::version v = ecos::library_version();
    return {v.major, v.minor, v.patch};
}

class my_listener: public ecos::simulation_listener {

public:
    explicit my_listener(ecos_simulation_listener_config config) {
        if (config.preStepCallback) {
            preStepCallback_ = config.preStepCallback;
        }
        if (config.postStepCallback) {
            postStepCallback_ = config.postStepCallback;
        }
    }

    void pre_step(ecos::simulation& sim) override
    {
        if (preStepCallback_) preStepCallback_->operator()(createInfo(sim));
    }

    void post_step(ecos::simulation& sim) override
    {
        if (postStepCallback_) postStepCallback_->operator()(createInfo(sim));
    }

private:
    std::optional<std::function<void(ecos_simulation_info)>> preStepCallback_;
    std::optional<std::function<void(ecos_simulation_info)>> postStepCallback_;

    static ecos_simulation_info createInfo(const ecos::simulation& sim) {
        return ecos_simulation_info{
            sim.time(),
            sim.iterations()
        };
    }
};

ecos_simulation_listener_t* ecos_simulation_listener_create(ecos_simulation_listener_config config)
{
    auto l = std::make_unique<ecos_simulation_listener_t>();
    l->cpp_listener = std::make_unique<my_listener>(config);

    return l.release();
}
