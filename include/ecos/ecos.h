
#ifndef LIBECOS_ECOS_H
#define LIBECOS_ECOS_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

const char* ecos_last_error_msg();


typedef struct ecos_simulation ecos_simulation_t;
typedef struct ecos_simulation_structure ecos_simulation_structure_t;


// system structure
ecos_simulation_structure_t* ecos_simulation_structure_create_from_ssp(const char* path);

void ecos_simulation_structure_destroy(ecos_simulation_structure_t* ss);


// simulation
ecos_simulation_t* ecos_simulation_create(ecos_simulation_structure_t* ss, double stepSize);

bool ecos_simulation_init(ecos_simulation_t* sim, double startTime = 0, const char* parameterSet = NULL);

void ecos_simulation_step(ecos_simulation_t* sim, size_t numSteps = 1);

void ecos_simulation_destroy(ecos_simulation_t* sim);



#ifdef __cplusplus
} // extern(C)
#endif

#endif // LIBECOS_ECOS_H
