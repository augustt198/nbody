#include "bh.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <cilk/cilk.h>

bh_node_t *new_empty_bh() {
    bh_node_t *node = malloc(sizeof(bh_node_t));
    for (int i = 0; i < NOCTANTS; i++) {
        node->octants[i] = NULL;
    }
    node->totalmass = 0.0;
    node->moment = vec3_const(0.0);
    node->points = NULL;
    node->numleaves = 0;

    return node;
}

bh_node_t *bh_create(int npts, point_mass_t *points) {
    bh_node_t *root = new_empty_bh();
    float maxcoord = -INFINITY;
    float mincoord = INFINITY;

    for (int i = 0; i < npts; i++) {
        for (int d = 0; d < 3; d++) {
            mincoord = fminf(mincoord, points[i].pos[d]);
            maxcoord = fmaxf(maxcoord, points[i].pos[d]);
        }
    }
    root->corner1 = vec3_const(mincoord*1.01);
    root->corner2 = vec3_const(maxcoord*1.01);

    for (int i = 0; i < npts; i++) {
        bh_insert(root, &points[i]);
    }

    return root;
}

void bh_destroy(bh_node_t *node) {
    for (int i = 0; i < NOCTANTS; i++) {
        if (node->octants[i] != NULL) {
            bh_destroy(node->octants[i]);
        }
    }
    if (node->points != NULL) {
        free(node->points);
    }
    free(node);
}

void bh_split(bh_node_t *node) {
    vec3 diag = 0.5*(node->corner2 - node->corner1);
    for (int i = 0; i < NOCTANTS; i++) {
        int b0 = i & 1;
        int b1 = (i>>1) & 1;
        int b2 = (i>>2) & 1;

        vec3 corner = node->corner1 + (vec3){b0, b1, b2} * diag;

        bh_node_t *octant = new_empty_bh();
        octant->corner1 = corner;
        octant->corner2 = corner + diag;
        node->octants[i] = octant;
    }
}

void bh_insert(bh_node_t *node, point_mass_t *pt) {
    if (node->octants[0] != NULL) {
        // was already split
        for (int i = 0; i < NOCTANTS; i++) {
            bh_node_t *octant = node->octants[i];
            bool inside = vec3_leq(octant->corner1, pt->pos) && vec3_lt(pt->pos, octant->corner2);
            if (inside) {
                cilk_spawn bh_insert(octant, pt);
                break;
            }
            cilk_sync;
        }
    } else {
        if (node->numleaves < MAXLEAVES) {
            if (node->points == NULL) {
                node->points = malloc(sizeof(point_mass_t*) * MAXLEAVES);
            }
            node->points[node->numleaves] = pt;
            node->numleaves++;
        }
        if (node->numleaves >= MAXLEAVES) { 
            // split it
            bh_split(node);
            for (int i = 0; i < node->numleaves; i++) {
                bh_insert(node, node->points[i]);
            }
            free(node->points);
            node->points = NULL;
        }
    }
}

void _bh_print(bh_node_t *node, int lvl, int *total) {
    for (int i = 0; i < lvl*2; i++) printf(" ");
    if (node->octants[0] == NULL) {
        printf("LEAF(%d) [%d]\n", lvl, node->numleaves);
        *total += node->numleaves;
    } else {
        printf("NODE(%d)\n", lvl);
    }
    
    for (int i = 0 ; i < NOCTANTS; i++) {
        if (node->octants[i] != NULL) {
            _bh_print(node->octants[i], lvl+1, total);
        }
    }
}

void bh_print(bh_node_t *node) {
    int total = 0;
    _bh_print(node, 0, &total);
    printf("TOTAL LEAVES: %d\n", total);
}

void bh_centers(bh_node_t *node) {
    vec3 moment = vec3_const(0.0);
    float totalmass = 0.0;
    for (int i = 0; i < NOCTANTS; i++) {
        if (node->octants[i] != NULL) {
            bh_centers(node->octants[i]);
            moment += node->octants[i]->moment;
            totalmass += node->octants[i]->totalmass;
        }
    }

    if (node->points != NULL) {
        for (int i = 0; i < node->numleaves; i++) {
            point_mass_t *pt = node->points[i];
            moment += pt->mass * pt->pos;
            totalmass += pt->mass;
        }
    }

    node->moment = moment;
    //printf("SETTING MOMENT [%f, %f, %f]\n", moment[0], moment[1], moment[2]);
    node->totalmass = totalmass;
}

#define THETA 0.90

vec3 bh_force(bh_node_t *node, point_mass_t *pt) {
    vec3 center = 0.5*(node->corner1 + node->corner2);
    float dist = vec3_norm(center - pt->pos);
    float width = (node->corner2 - node->corner1)[0];
    float ratio = width / dist;

    if (ratio < THETA) {
        if (node->totalmass == 0.0) {
            return vec3_const(0.0);
        }
        vec3 com = node->moment / node->totalmass;
        vec3 r = com - pt->pos;
        //vec3 force = pt->mass * node->totalmass * r / powf(vec3_normsq(r), 1.5);
        float nn = vec3_norm(r);
        vec3 force = pt->mass * node->totalmass * r / (nn * nn * nn);
        //printf("COM: [%f, %f, %f]\n", com[0], com[1], com[2]);
        return force;
    } else if (node->octants[0] == NULL) {
        vec3 total = vec3_const(0.0);
        for (int i = 0; i < node->numleaves; i++) {
            point_mass_t *pt2 = node->points[i];
            if (vec3_eq(pt->pos, pt2->pos)) {
                //printf("same point! skipping\n");
                continue;
            }
            vec3 r = pt2->pos - pt->pos;
            //total += pt->mass * pt2->mass * r / powf(vec3_normsq(r), 1.5);
            float nn = vec3_norm(r);
            total += pt->mass * pt2->mass * r / (nn * nn * nn);
        }
        //printf("TOTALA: [%f, %f, %f]\n", total[0], total[1], total[2]);
        return total;
    } else {
        vec3 total = vec3_const(0.0);
        for (int i = 0; i < NOCTANTS; i++) {
            total += bh_force(node->octants[i], pt);
        }
        //printf("TOTAL: [%f, %f, %f]\n", total[0], total[1], total[2]);
        return total;
    }
}
