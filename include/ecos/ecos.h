
#ifndef LIBECOS_ECOS_H
#define LIBECOS_ECOS_H

#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ecos_simulation ecos_simulation_t;
typedef struct ecos_simulation_runner ecos_simulation_runner_t;
typedef struct ecos_simulation_listener ecos_simulation_listener_t;
typedef struct ecos_simulation_structure ecos_simulation_structure_t;
typedef struct ecos_parameter_set ecos_parameter_set_t;

const char* ecos_last_error_msg();

void ecos_set_log_level(const char* level);

// simulation_structure
ecos_simulation_structure_t* ecos_simulation_structure_create();
void ecos_simulation_structure_destroy(ecos_simulation_structure_t* ss);

bool ecos_simulation_structure_add_model(ecos_simulation_structure_t* ss, const char* instanceName, const char* uri);
bool ecos_simulation_structure_add_parameter_set(ecos_simulation_structure_t* ss, const char* name, const ecos_parameter_set_t* pps);

void ecos_simulation_structure_make_int_connection(ecos_simulation_structure_t* ss, const char* source, const char* sink);
void ecos_simulation_structure_make_real_connection(ecos_simulation_structure_t* ss, const char* source, const char* sink);
void ecos_simulation_structure_make_real_connection_mod(ecos_simulation_structure_t* ss, const char* source, const char* sink, double (*modifier)(double) = nullptr);
void ecos_simulation_structure_make_string_connection(ecos_simulation_structure_t* ss, const char* source, const char* sink);
void ecos_simulation_structure_make_bool_connection(ecos_simulation_structure_t* ss, const char* source, const char* sink);
// -------------

// parameter_set
ecos_parameter_set_t* ecos_parameter_set_create();
void ecos_parameter_set_destroy(ecos_parameter_set_t* parameter_set);

void ecos_parameter_set_add_int(ecos_parameter_set_t* pps, const char* name, int value);
void ecos_parameter_set_add_real(ecos_parameter_set_t* pps, const char* name, double value);
void ecos_parameter_set_add_string(ecos_parameter_set_t* pps, const char* name, const char* value);
void ecos_parameter_set_add_bool(ecos_parameter_set_t* pps, const char* name, bool value);
// -------------

// simulation
ecos_simulation_t* ecos_simulation_create_from_ssp(const char* sspPath, double stepSize);
ecos_simulation_t* ecos_simulation_create_from_structure(ecos_simulation_structure_t* structure, double stepSize);

bool ecos_simulation_init(ecos_simulation_t* sim, double startTime = 0, const char* parameterSet = nullptr);

double ecos_simulation_step(ecos_simulation_t* sim, size_t numSteps = 1);
void ecos_simulation_step_for(ecos_simulation_t* sim, double duration);
void ecos_simulation_step_until(ecos_simulation_t* sim, double timePoint);

bool ecos_simulation_get_integer(ecos_simulation_t* sim, const char* identifier, int* value);
bool ecos_simulation_get_real(ecos_simulation_t* sim, const char* identifier, double* value);
bool ecos_simulation_get_bool(ecos_simulation_t* sim, const char* identifier, bool* value);
bool ecos_simulation_get_string(ecos_simulation_t* sim, const char* identifier, char* value, size_t value_size);

bool ecos_simulation_set_integer(ecos_simulation_t* sim, const char* identifier, int value);
bool ecos_simulation_set_real(ecos_simulation_t* sim, const char* identifier, double value);
bool ecos_simulation_set_bool(ecos_simulation_t* sim, const char* identifier, bool value);
bool ecos_simulation_set_string(ecos_simulation_t* sim, const char* identifier, const char* value);

bool ecos_simulation_terminate(ecos_simulation_t* sim);
bool ecos_simulation_reset(ecos_simulation_t* sim);
void ecos_simulation_destroy(ecos_simulation_t* sim);

bool ecos_simulation_load_scenario(ecos_simulation_t* sim, const char* scenario_file);
// -------------

// simulation_runner
ecos_simulation_runner_t* ecos_simulation_runner_create(ecos_simulation_t* sim);
void ecos_simulation_runner_start(const ecos_simulation_runner_t* runner);
void ecos_simulation_runner_stop(const ecos_simulation_runner_t* runner);
void ecos_simulation_runner_set_real_time_factor(const ecos_simulation_runner_t* runner, double factor);
void ecos_simulation_runner_destroy(const ecos_simulation_runner_t* runner);
// -------------

// simulation_listener
typedef struct ecos_simulation_info
{
    double time;
    size_t iterations;
} ecos_simulation_info;

typedef struct ecos_simulation_listener_config
{
    void (*preStepCallback)(ecos_simulation_info) = nullptr;
    void (*postStepCallback)(ecos_simulation_info) = nullptr;
} ecos_simulation_listener_config;

ecos_simulation_listener_t* ecos_simulation_listener_create(ecos_simulation_listener_config config);

void ecos_simulation_add_listener(ecos_simulation_t* sim, const char* name, ecos_simulation_listener_t* listener);
void ecos_simulation_remove_listener(ecos_simulation_t* sim, const char* name);
ecos_simulation_listener_t* ecos_csv_writer_create(const char* resultFile, const char* csvConfig = nullptr);
bool ecos_csv_writer_set_decimation_factor(ecos_simulation_listener_t* writer, int decimationFactor);

void ecos_plot_csv(const char* csvFile, const char* chartConfig);
// -------------

// version
typedef struct ecos_version
{
    int major;
    int minor;
    int patch;
} ecos_version;

ecos_version ecos_library_version();
// -------------


#ifdef __cplusplus
} // extern(C)
#endif

#endif // LIBECOS_ECOS_H
