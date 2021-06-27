#include "vector.h"
#include <math.h>

vec3 vec3_const(float c) {
    return (vec3){c, c, c};
}

float vec3_norm(vec3 v) {
    return sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

float vec3_normsq(vec3 v) {
    return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}

float vec3_dot(vec3 u, vec3 v) {
    return u[0]*v[0] + u[1]*v[1] + u[2]*v[2];
}

vec3 vec3_cross(vec3 u, vec3 v) {
    vec3 w;
    w[0] = u[1]*v[2] - u[2]*v[1];
    w[1] = -u[0]*v[2] + u[2]*v[0];
    w[2] = u[0]*v[1] - u[1]*v[0];
    return w;
}

vec3 vec3_normalize(vec3 v) {
    return v / vec3_norm(v);
}

bool vec3_eq(vec3 u, vec3 v) {
    return u[0] == v[0] && u[1] == v[1] && u[2] == v[2];
}
