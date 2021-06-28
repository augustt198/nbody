#include "simulation.h"

#include <math.h>
#include "bh.h"

void simulate_step(simulation_setup_t *sim) {
    float G = sim->Gconst;
    int N = sim->n_points;

    /*
    // accumulate acceleration
    for (int i = 0; i < N; i++) {
        sim->points[i].accel = vec3_const(0.0);
        vec3 accel = vec3_const(0.0);
        for (int j = 0; j < N; j++) {
            if (i == j) continue;

            vec3 r = sim->points[j].pos - sim->points[i].pos;
            float m2 = sim->points[j].mass;

            //sim->points[i].accel += G * m2 * r / powf(norm(r), 3);
            accel += G * m2 * vec3_normalize(r) / (vec3_normsq(r) + 1e-10f);
        }
        sim->points[i].accel = accel;
    }
    */

    bh_node_t *bh = bh_create(N, sim->points);
    bh_centers(bh);
    for (int i = 0; i < N; i++) {
        point_mass_t *pt = &sim->points[i];
        pt->accel = G * bh_force(bh, pt) / pt->mass;
    }
    bh_destroy(bh);

    float dt = sim->timestep;

    // update positions
    for (int i = 0; i < N; i++) {
        sim->points[i].pos += sim->points[i].vel * dt;
    }

    // update velocities
    for (int i = 0; i < N; i++) {
        sim->points[i].vel += sim->points[i].accel * dt;
    }
}

void setup_leapfrog(simulation_setup_t *sim) {
    float dt = sim->timestep;
    for (int i = 0; i < sim->n_points; i++) {
        sim->points[i].vel += sim->points[i].accel[i] * dt * 0.5f;
    }
}

float kinetic_energy(simulation_setup_t *sim) {
    float total = 0.0;
    for (int i = 0; i < sim->n_points; i++) {
        total += sim->points[i].mass * vec3_normsq(sim->points[i].vel);
    }

    return 0.5 * total;
}

float potential_energy(simulation_setup_t *sim) {
    float G = sim->Gconst;
    int N = sim->n_points;

    float total = 0.0;

    // accumulate acceleration
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == j) continue;

            vec3 r = sim->points[i].pos - sim->points[j].pos;
            float m1 = sim->points[i].mass, m2 = sim->points[j].mass;

            total += -G * m1 * m2 / vec3_norm(r);
        }
    }

    return 0.5 * total;
}
