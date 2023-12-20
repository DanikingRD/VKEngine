#ifndef XMATH_H
#define XMATH_H

#include "defines.h"
#include "lineal_types.h"
#include "types.h"
#include <math.h>

// +-----------------------+
// + Vector 2 Functions    +
// +-----------------------+

INLINE Vec2 vec2_create(f32 x, f32 y) {
    Vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

INLINE Vec2 vec2_zero(void) { return (Vec2){0.0f, 0.0f}; }

INLINE Vec2 vec2_add(Vec2 a, Vec2 b) {
    return (Vec2){
        a.x + b.x,
        a.y + b.y,
    };
}

INLINE Vec2 vec2_sub(Vec2 a, Vec2 b) {
    return (Vec2){
        a.x - b.x,
        a.y - b.y,
    };
}

// +-----------------------+
// + Vector 3 Functions    +
// +-----------------------+

INLINE Vec3 vec3_create(f32 x, f32 y, f32 z) {
    return (Vec3){
        x,
        y,
        z,
    };
}

INLINE Vec3 vec3_zero(void) {
    return (Vec3){
        0.0f,
        0.0f,
        0.0f,
    };
}
INLINE Vec3 vec3_one(void) {
    return (Vec3){
        1.0f,
        1.0f,
        1.0f,
    };
}

INLINE Vec3 vec3_add(Vec3 a, Vec3 b) {
    return (Vec3){
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
    };
}

INLINE Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return (Vec3){
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
    };
}

INLINE f32 vec3_length_squared(Vec3 v) { return v.x * v.x + v.y * v.y + v.z * v.z; }

INLINE f32 vec3_length(Vec3 v) { return sqrt(vec3_length_squared(v)); }

INLINE void vec3_normalize(Vec3* v) {
    const f32 l = vec3_length(*v);
    v->x /= l;
    v->y /= l;
    v->z /= l;
}

INLINE Vec3 vec3_normalized(Vec3 v) {
    vec3_normalize(&v);
    return v;
}

#endif
