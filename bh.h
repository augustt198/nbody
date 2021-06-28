#include "vector.h"
#include "simulation.h"

#define NOCTANTS 8
#define MAXLEAVES 64

typedef enum {
    Q_I = 0, Q_II, Q_III, Q_IV,
    Q_V, Q_VI, Q_VII, Q_VIII
} octant_label_t;

typedef struct bh_node_t {
    vec3 corner1, corner2;
    struct bh_node_t *octants[NOCTANTS];
    vec3 moment;
    float totalmass;
    point_mass_t **points;
    int numleaves;
} bh_node_t;

bh_node_t *bh_create(int npts, point_mass_t *points);
void bh_destroy(bh_node_t *node);
void bh_insert(bh_node_t *node, point_mass_t *pt);
void bh_print(bh_node_t *node);
vec3 bh_force(bh_node_t *node, point_mass_t *pt);
