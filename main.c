#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "vector.h"
#include "simulation.h"
#include "graphics.h"
#include "bh.h"

void setup_accel(simulation_setup_t *sim) {
    float G = sim->Gconst;
    int N = sim->n_points;

    // accumulate acceleration
    for (int i = 0; i < N; i++) {
        sim->points[i].accel = vec3_const(0.0);
        vec3 accel = vec3_const(0.0);
        for (int j = 0; j < N; j++) {
            if (i == j) continue;

            vec3 r = sim->points[j].pos - sim->points[i].pos;
            float m2 = sim->points[j].mass;

            //sim->points[i].accel += G * m2 * r / powf(norm(r), 3);
            accel += G * m2 * vec3_normalize(r) / vec3_normsq(r);
        }
        sim->points[i].accel = accel;
    }
}

#define NPTS 15000

float randf() {
    return (float) rand() / RAND_MAX;
}

int main(int argc, char **argv) {
    srand(0);

    //point_mass_t points[NPTS];
    point_mass_t *points = malloc(sizeof(point_mass_t) * NPTS);

    for (int i = 0; i < NPTS; i++) {
        bool part = i > NPTS/2;
        float theta = randf() * 2 * 3.141569;
        float radius = 2.0 + randf() * 0.25;
        radius += part ? 1.0 : 0.0;
        points[i].pos = 2.0f * (vec3){cosf(theta)*radius, sinf(theta)*radius, randf()-0.5};

        points[i].vel = 100.f * (vec3){-points[i].pos[1], points[i].pos[0], 0.0};
        if (part) points[i].vel *= -0.7f;
        points[i].accel = vec3_const(0.0);
        points[i].mass = 0.1;
    }

    points[0].pos = vec3_const(0.0);
    points[0].vel = vec3_const(0.0);
    points[0].mass = 10000.0;

    bh_node_t *bh = bh_create(NPTS, points);
    for (int i = 0; i < NOCTANTS; i++) {
        vec3 a = bh->octants[i]->corner1;
        vec3 b = bh->octants[i]->corner2;
        printf("Q%d [%.2f, %.2f, %.2f] - [%.2f, %.2f, %.2f]\n", i,
            a[0], a[1], a[2], b[0], b[1], b[2]);
    }

    bh_print(bh);
    bh_centers(bh);
    printf("total mass: %f\n", bh->totalmass);

    vec3 com = bh->moment / bh->totalmass;



    // points[0].accel = vec3_const(0.0);
    // points[0].vel = (vec3){0.0, 1.0, 0.0};
    // points[0].pos = (vec3){2.0, 0.0, 0.0};
    // points[0].mass = 1.0;

    // points[1].accel = vec3_const(0.0);
    // points[1].vel = (vec3){0.0, -1.0, 0.0};
    // points[1].pos = (vec3){-2.0, 0.0, 0.0};
    // points[1].mass = 1.0;

    simulation_setup_t sim;
    sim.Gconst = 100.0;
    sim.n_points = NPTS;
    sim.points = points;
    sim.timestep = 0.0001;

    //setup_accel(&sim);
    //setup_leapfrog(&sim);

    int num = 2;
    vec3 force = sim.Gconst * bh_force(bh, &points[num]) / points[num].mass;
    printf("$$$ FORCE: [%f, %f, %f]\n", force[0], force[1], force[2]);

    simulate_step(&sim);
    printf("THE ACCEL: [%f, %f, %f]\n", points[num].accel[0], points[num].accel[1], points[num].accel[2]);

    //return 0;


    float Emin = INFINITY, Emax = -INFINITY;

    if (!graphics_init(&sim)) {
        goto finish;
    }

    while (true) {
        if (!graphics_update(&sim)) {
            goto finish;
        }

        simulate_step(&sim);
        float Ek = kinetic_energy(&sim);
        float Ep = 0.0; // potential_energy(&sim);
        printf("Energy: %.4f + %.4f = %.4f\n", Ek, Ep, Ek+Ep);
    }

    // for (int i = 0; i < 100000; i++) {
    //     float Ek = kinetic_energy(&sim);
    //     float Ep = potential_energy(&sim);
    //     float E = Ek + 0.5*Ep;
    //     Emin = fminf(Emin, E);
    //     Emax = fmaxf(Emax, E);
    //     printf("[%d] P1 [%.3f, %.3f, %.3f] // P2 [%.3f, %.3f, %.3f]    %f %f %f [%f ~ %f]\n", i,
    //         points[0].pos[0], points[0].pos[1], points[0].pos[2],
    //         points[1].pos[0], points[1].pos[1], points[1].pos[2],
    //         Ek, Ep, E,
    //         Emin, Emax);

    //     simulate_step(&sim);

    //     if (fabsf(E + 24.0f) > 0.01) break;
    // }

    finish:
    graphics_exit();

    return 0;
}
