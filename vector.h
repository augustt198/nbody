#pragma once

#include <stdbool.h>

typedef float float4 __attribute__ ((vector_size (16)));
// just ignore the fourth component :)
typedef float4 vec3;

vec3 vec3_const(float c);

// The Euclidian norm ||v||
float vec3_norm(vec3 v);

// The squared Euclidian norm ||v||^2
float vec3_normsq(vec3 v);

// Dot product of u and v
float vec3_dot(vec3 u, vec3 v);

// Cross product of u and v
vec3 vec3_cross(vec3 u, vec3 v);

// Returns v / ||v||
vec3 vec3_normalize(vec3 v);

bool vec3_eq(vec3 u, vec3 v);
