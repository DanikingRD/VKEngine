#ifndef LINEAL_TYPES
#define LINEAL_TYPES

#include "types.h"

typedef union Vec2 {
    f32 data[2];
    struct {
        union {
            f32 x, u;
        };
        union {
            f32 y, v;
        };
    };
} Vec2;

typedef union Vec3 {
    f32 data[3];
    struct {
        union {
            f32 x, r;
        };
        union {
            f32 y, g;
        };
        union {
            f32 z, b;
        };
    };
} Vec3;

typedef union Mat4 {
    f32 data[16];
} Mat4;

#endif
