
#include "ecos/ecos.h"

#include "ecos/algorithm/fixed_step_algorithm.hpp"
#include "ecos/lib_info.hpp"
#include "ecos/listeners/csv_writer.hpp"
#include "ecos/logger/logger.hpp"
#include "ecos/simulation.hpp"
#include "ecos/simulation_runner.hpp"
#include "ecos/ssp/ssp_loader.hpp"
#include "ecos/util/plotter.hpp"

#include <cstring>
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
    ecos::simulation_structure cpp_ss;
};

struct ecos_simulation_runner
{

    explicit ecos_simulation_runner(ecos_simulation* sim)
    {
        cpp_runner = std::make_unique<ecos::simulation_runner>(*sim->cpp_sim);
    }
    std::unique_ptr<ecos::simulation_runner> cpp_runner;
};

struct ecos_simulation_listener
{
    std::unique_ptr<ecos::simulation_listener> cpp_listener;
};

struct ecos_parameter_set
{
    ecos::parameter_set cpp_parameter_set;
};

void handle_current_exception()
{
    try {
        throw;
    } catch (const std::exception& ex) {
        g_last_error_msg = ex.what();
    }
}

void ecos_set_log_level(const char* level)
{
    if (std::string("trace") == level) {
        set_logging_level(ecos::log::level::trace);
    } else if (std::string("debug") == level) {
        set_logging_level(ecos::log::level::debug);
    } else if (std::string("info") == level) {
        set_logging_level(ecos::log::level::info);
    } else if (std::string("warn") == level) {
        set_logging_level(ecos::log::level::warn);
    } else if (std::string("err") == level) {
        set_logging_level(ecos::log::level::err);
    } else if (std::string("off") == level) {
        set_logging_level(ecos::log::level::off);
    }
}

ecos_simulation_structure_t* ecos_simulation_structure_create()
{
    try {
        auto ss = std::make_unique<ecos_simulation_structure_t>();
        return ss.release();
    } catch (...) {
        handle_current_exception();
        return nullptr;
    }
}

void ecos_simulation_structure_destroy(ecos_simulation_structure_t* ss)
{
    if (ss) {
        delete ss;
        ss = nullptr;
    }
}


ecos_parameter_set_t* ecos_parameter_set_create()
{
    try {
        auto pps = std::make_unique<ecos_parameter_set_t>();
        return pps.release();
    } catch (...) {
        handle_current_exception();
        return nullptr;
    }
}

void ecos_parameter_set_destroy(ecos_parameter_set_t* parameter_set)
{
    if (parameter_set) {
        delete parameter_set;
        parameter_set = nullptr;
    }
}

void ecos_parameter_set_add_int(ecos_parameter_set_t* pps, const char* name, int value)
{
    try {
        pps->cpp_parameter_set[name] = value;
    } catch (...) {
        handle_current_exception();
    }
}

void ecos_parameter_set_add_real(ecos_parameter_set_t* pps, const char* name, double value)
{
    try {
        pps->cpp_parameter_set[name] = value;
    } catch (...) {
        handle_current_exception();
    }
}

void ecos_parameter_set_add_string(ecos_parameter_set_t* pps, const char* name, const char* value)
{
    try {
        pps->cpp_parameter_set[name] = value;
    } catch (...) {
        handle_current_exception();
    }
}

void ecos_parameter_set_add_bool(ecos_parameter_set_t* pps, const char* name, bool value)
{
    try {
        pps->cpp_parameter_set[name] = value;
    } catch (...) {
        handle_current_exception();
    }
}

bool ecos_simulation_structure_add_model(ecos_simulation_structure_t* ss, const char* instanceName, const char* uri)
{
    try {
        ss->cpp_ss.add_model(instanceName, std::string(uri));
        return true;
    } catch (...) {
        handle_current_exception();
        return false;
    }
}

bool ecos_simulation_structure_add_parameter_set(ecos_simulation_structure_t* ss, const char* name, const ecos_parameter_set_t* pps)
{
    try {
        ss->cpp_ss.add_parameter_set(name, pps->cpp_parameter_set);
        return true;
    } catch (...) {
        handle_current_exception();
        return false;
    }
}

void ecos_simulation_structure_make_int_connection(ecos_simulation_structure_t* ss, const char* source, const char* sink)
{
    try {
        ss->cpp_ss.make_connection<int>(source, sink);
    } catch (...) {
        handle_current_exception();
    }
}

void ecos_simulation_structure_make_real_connection(ecos_simulation_structure_t* ss, const char* source, const char* sink)
{
    try {
        ss->cpp_ss.make_connection<double>(source, sink);
    } catch (...) {
        handle_current_exception();
    }
}

void ecos_simulation_structure_make_real_connection_mod(ecos_simulation_structure_t* ss, const char* source, const char* sink, double (*modifier)(double))
{
    try {
        ss->cpp_ss.make_connection<double>(source, sink, [modifier](double value) { return modifier(value); });
    } catch (...) {
        handle_current_exception();
    }
}

void ecos_simulation_structure_make_string_connection(ecos_simulation_structure_t* ss, const char* source, const char* sink)
{
    try {
        ss->cpp_ss.make_connection<std::string>(source, sink);
    } catch (...) {
        handle_current_exception();
    }
}

void ecos_simulation_structure_make_bool_connection(ecos_simulation_structure_t* ss, const char* source, const char* sink)
{
    try {
        ss->cpp_ss.make_connection<bool>(source, sink);
    } catch (...) {
        handle_current_exception();
    }
}

ecos_simulation_t* ecos_simulation_create_from_ssp(const char* sspPath, double stepSize)
{
    try {
        const auto ss = ecos::load_ssp(sspPath);

        auto algorithm = std::make_unique<ecos::fixed_step_algorithm>(stepSize);
        auto sim = std::make_unique<ecos_simulation_t>();
        sim->cpp_sim = ss->load(std::move(algorithm));
        return sim.release();
    } catch (...) {
        handle_current_exception();
        return nullptr;
    }
}

ecos_simulation_t* ecos_simulation_create_from_structure(ecos_simulation_structure_t* ss, double stepSize)
{
    try {

        auto sim = std::make_unique<ecos_simulation_t>();
        auto algorithm = std::make_unique<ecos::fixed_step_algorithm>(stepSize);
        sim->cpp_sim = ss->cpp_ss.load(std::move(algorithm));

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

double ecos_simulation_step(ecos_simulation_t* sim, size_t numSteps)
{
    return sim->cpp_sim->step(numSteps);
}

void ecos_simulation_step_for(ecos_simulation_t* sim, double duration)
{
    sim->cpp_sim->step_for(duration);
}

void ecos_simulation_step_until(ecos_simulation_t* sim, double timePoint)
{
    sim->cpp_sim->step_until(timePoint);
}

bool ecos_simulation_get_integer(ecos_simulation_t* sim, const char* identifier, int* value)
{
    try {
        const auto prop = sim->cpp_sim->get_int_property(identifier);
        if (!prop) {
            g_last_error_msg = "No int property " + std::string(identifier) + " found!";
            return false;
        }
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
        const auto prop = sim->cpp_sim->get_real_property(identifier);
        if (!prop) {
            g_last_error_msg = "No real property " + std::string(identifier) + " found!";
            return false;
        }
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
        const auto prop = sim->cpp_sim->get_bool_property(identifier);
        if (!prop) {
            g_last_error_msg = "No bool property " + std::string(identifier) + " found!";
            return false;
        }
        *value = prop->get_value();
        return true;
    } catch (...) {
        handle_current_exception();
        return false;
    }
}

bool ecos_simulation_get_string(ecos_simulation_t* sim, const char* identifier, char* value)
{
    try {
        const auto prop = sim->cpp_sim->get_string_property(identifier);
        if (!prop) {
            g_last_error_msg = "No string property named" + std::string(identifier) + " found!";
            return false;
        }
        const auto propValue = prop->get_value();
        std::strcpy(value, propValue.c_str());

        return true;
    } catch (...) {
        handle_current_exception();
        return false;
    }
}

bool ecos_simulation_set_integer(ecos_simulation_t* sim, const char* identifier, int value)
{
    try {
        const auto prop = sim->cpp_sim->get_int_property(identifier);
        if (!prop) {
            g_last_error_msg = "No int property " + std::string(identifier) + " found!";
            return false;
        }
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
        const auto prop = sim->cpp_sim->get_real_property(identifier);
        if (!prop) {
            g_last_error_msg = "No real property " + std::string(identifier) + " found!";
            return false;
        }
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
        const auto prop = sim->cpp_sim->get_bool_property(identifier);
        if (!prop) {
            g_last_error_msg = "No bool property " + std::string(identifier) + " found!";
            return false;
        }
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
        const auto prop = sim->cpp_sim->get_string_property(identifier);
        if (!prop) {
            g_last_error_msg = "No string property " + std::string(identifier) + " found!";
            return false;
        }
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

ecos_simulation_listener_t* ecos_csv_writer_create(const char* resultFile, const char* csvConfig)
{
    try {

        auto writer = std::make_unique<ecos::csv_writer>(resultFile);
        if (csvConfig) {
            writer->config().load(csvConfig);
        }

        auto l = std::make_unique<ecos_simulation_listener_t>();
        l->cpp_listener = std::move(writer);

        return l.release();

    } catch (...) {
        handle_current_exception();
        return nullptr;
    }
}

bool ecos_csv_writer_set_decimation_factor(ecos_simulation_listener_t* writer, int decimationFactor)
{
    try {

        const auto csv_writer = dynamic_cast<ecos::csv_writer*>(writer->cpp_listener.get());
        csv_writer->config().decimation_factor() = decimationFactor;
        return true;

    } catch (...) {
        handle_current_exception();
        return false;
    }
}

void ecos_plot_csv(const char* csvFile, const char* chartConfig)
{
    ecos::plot_csv(csvFile, chartConfig);
}

bool ecos_simulation_terminate(ecos_simulation_t* sim)
{
    try {
        sim->cpp_sim->terminate();
    } catch (...) {
        handle_current_exception();
        return false;
    }
    return true;
}


bool ecos_simulation_reset(ecos_simulation_t* sim)
{
    try {
        sim->cpp_sim->reset();
    } catch (...) {
        handle_current_exception();
        return false;
    }
    return true;
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

class my_listener : public ecos::simulation_listener
{

public:
    explicit my_listener(ecos_simulation_listener_config config)
    {
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

    static ecos_simulation_info createInfo(const ecos::simulation& sim)
    {
        return ecos_simulation_info{
            sim.time(),
            sim.iterations()};
    }
};

ecos_simulation_listener_t* ecos_simulation_listener_create(ecos_simulation_listener_config config)
{
    auto l = std::make_unique<ecos_simulation_listener_t>();
    l->cpp_listener = std::make_unique<my_listener>(config);

    return l.release();
}

bool ecos_simulation_load_scenario(ecos_simulation_t* sim, const char* scenario_file)
{
    try {
        sim->cpp_sim->load_scenario(scenario_file);
        return true;
    } catch (...) {
        handle_current_exception();
        return false;
    }
}

ecos_simulation_runner_t* ecos_simulation_runner_create(ecos_simulation_t* sim)
{
    auto runner = std::make_unique<ecos_simulation_runner_t>(sim);

    return runner.release();
}

void ecos_simulation_runner_start(const ecos_simulation_runner_t* runner)
{
    runner->cpp_runner->start();
}

void ecos_simulation_runner_stop(const ecos_simulation_runner_t* runner)
{
    runner->cpp_runner->stop();
}

void ecos_simulation_runner_set_real_time_factor(const ecos_simulation_runner_t* runner, double factor)
{
    runner->cpp_runner->set_real_time_factor(factor);
}

void ecos_simulation_runner_destroy(const ecos_simulation_runner_t* runner)
{
    if (runner) {
        delete runner;
        runner = nullptr;
    }
}
