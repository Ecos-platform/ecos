
#ifndef LIBECOS_ECOS_H
#define LIBECOS_ECOS_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

const char* ecos_last_error_msg();


typedef struct ecos_simulation ecos_simulation_t;


ecos_simulation_t* ecos_simulation_create(const char* sspPath, double stepSize);

bool ecos_simulation_init(ecos_simulation_t* sim, double startTime = 0, const char* parameterSet = NULL);

void ecos_simulation_step(ecos_simulation_t* sim, size_t numSteps = 1);

bool ecos_simulation_add_csv_writer(ecos_simulation_t* sim, const char* resultFile, const char* configFile = NULL);

void ecos_simulation_destroy(ecos_simulation_t* sim);


typedef struct
{
    int major;
    int minor;
    int patch;
} ecos_version;

ecos_version ecos_library_version();


#ifdef __cplusplus
} // extern(C)
#endif

#endif // LIBECOS_ECOS_H
