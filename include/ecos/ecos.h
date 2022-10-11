
#ifndef LIBECOS_ECOS_H
#define LIBECOS_ECOS_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

const char* ecos_last_error_msg();

struct simlation_listener_config {
    void (*preStepCallback)() = nullptr;
    void (*postStepCallback)() = nullptr;
};

typedef struct ecos_simulation ecos_simulation_t;
typedef struct ecos_simulation_listener ecos_simulation_listener_t;

void set_log_level(const char* level);

ecos_simulation_t* ecos_simulation_create(const char* sspPath, double stepSize);

bool ecos_simulation_init(ecos_simulation_t* sim, double startTime = 0, const char* parameterSet = nullptr);

void ecos_simulation_step(ecos_simulation_t* sim, size_t numSteps = 1);
void ecos_simulation_step_until(ecos_simulation_t* sim, double timePoint);

bool ecos_simulation_get_integer(ecos_simulation_t* sim, const char* identifier, int* value);
bool ecos_simulation_get_real(ecos_simulation_t* sim, const char* identifier, double* value);
bool ecos_simulation_get_bool(ecos_simulation_t* sim, const char* identifier, bool* value);
bool ecos_simulation_get_string(ecos_simulation_t* sim, const char* identifier, const char* value);

bool ecos_simulation_set_integer(ecos_simulation_t* sim, const char* identifier, int value);
bool ecos_simulation_set_real(ecos_simulation_t* sim, const char* identifier, double value);
bool ecos_simulation_set_bool(ecos_simulation_t* sim, const char* identifier, bool value);
bool ecos_simulation_set_string(ecos_simulation_t* sim, const char* identifier, const char* value);

void ecos_simulation_terminate(ecos_simulation_t* sim);

void ecos_simulation_add_listener(ecos_simulation_t* sim, const char* name, ecos_simulation_listener_t* listener);
void ecos_simulation_remove_listener(ecos_simulation_t* sim, const char* name);
ecos_simulation_listener_t* ecos_csv_writer_create(const char* resultFile, const char* logConfig = nullptr, const char* plotConfig = nullptr);
ecos_simulation_listener_t* ecos_listener_create(simlation_listener_config config);

void ecos_simulation_destroy(ecos_simulation_t* sim);


typedef struct
{
    int major;
    int minor;
    int patch;
} ecos_version;

ecos_version* ecos_library_version();


#ifdef __cplusplus
} // extern(C)
#endif

#endif // LIBECOS_ECOS_H
