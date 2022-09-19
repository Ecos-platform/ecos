
#ifndef LIBECOS_ECOS_H
#define LIBECOS_ECOS_H

#ifdef __cplusplus
extern "C" {
#endif

const char* last_error_msg();

struct ecos_simulation_s;

///// An opaque object which contains the state for an simulation.
//typedef struct ecos_simulation_s ecos_simulation;

struct ecos_simulation_structure_s;

///// An opaque object which contains the state for an simulation structure.
//typedef struct ecos_simulation_structure_s ecos_simulation_structure;

ecos_simulation_structure_s* ecos_simulation_structure_create_from_ssp(const char* path);

void ecos_simulation_structure_destroy(ecos_simulation_structure_s* ss);

ecos_simulation_s* ecos_simulation_create(ecos_simulation_structure_s* ss, double stepSize);

void ecos_simulation_destroy(ecos_simulation_s*);

void ecos_simulation_step(ecos_simulation_s* sim, size_t numSteps = 1);

#ifdef __cplusplus
} // extern(C)
#endif

#endif // LIBECOS_ECOS_H
