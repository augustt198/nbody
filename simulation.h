#pragma once

#include "vector.h"

typedef struct  {
    vec3 pos;
    vec3 vel;
    vec3 accel;
    float mass;
} point_mass_t;

typedef struct {
    int n_points;
    point_mass_t *points;
    float timestep;
    float Gconst;
} simulation_setup_t;


void simulate_step(simulation_setup_t *sim);
void setup_leapfrog(simulation_setup_t *sim);
float kinetic_energy(simulation_setup_t *sim);
float potential_energy(simulation_setup_t *sim);
