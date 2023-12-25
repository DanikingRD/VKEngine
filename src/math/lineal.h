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

INLINE f32 vec3_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

INLINE Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y, //
        a.z * b.x - a.x * b.z, //
        a.x * b.y - a.y * b.x, //
    };
}

INLINE Mat4 mat4_identity(void) {
    return (Mat4){
        1.0f, 0.0f, 0.0f, 0.0f, //
        0.0f, 1.0f, 0.0f, 0.0f, //
        0.0f, 0.0f, 1.0f, 0.0f, //
        0.0f, 0.0f, 0.0f, 1.0f, //
    };
}

INLINE Mat4 mat4_mul(Mat4 matrix_0, Mat4 matrix_1) {
    Mat4 out_matrix = mat4_identity();

    const f32* m1_ptr = matrix_0.data;
    const f32* m2_ptr = matrix_1.data;
    f32* dst_ptr = out_matrix.data;

    for (i32 i = 0; i < 4; ++i) {
        for (i32 j = 0; j < 4; ++j) {
            *dst_ptr = m1_ptr[0] * m2_ptr[0 + j] + m1_ptr[1] * m2_ptr[4 + j] +
                       m1_ptr[2] * m2_ptr[8 + j] + m1_ptr[3] * m2_ptr[12 + j];
            dst_ptr++;
        }
        m1_ptr += 4;
    }
    return out_matrix;
}

INLINE Mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 near_clip, f32 far_clip) {
    f32 half_tan_fov = tanf(fov_radians * 0.5f);
    Mat4 out_matrix = {0};
    out_matrix.data[0] = 1.0f / (aspect_ratio * half_tan_fov);
    out_matrix.data[5] = 1.0f / half_tan_fov;
    out_matrix.data[10] = -((far_clip + near_clip) / (far_clip - near_clip));
    out_matrix.data[11] = -1.0f;
    out_matrix.data[14] = -((2.0f * far_clip * near_clip) / (far_clip - near_clip));
    return out_matrix;
}

INLINE Mat4 mat4_look_at(Vec3 position, Vec3 target, Vec3 up) {
    Mat4 out_matrix;
    Vec3 z_axis;
    z_axis.x = target.x - position.x;
    z_axis.y = target.y - position.y;
    z_axis.z = target.z - position.z;

    z_axis = vec3_normalized(z_axis);
    Vec3 x_axis = vec3_normalized(vec3_cross(z_axis, up));
    Vec3 y_axis = vec3_cross(x_axis, z_axis);

    out_matrix.data[0] = x_axis.x;
    out_matrix.data[1] = y_axis.x;
    out_matrix.data[2] = -z_axis.x;
    out_matrix.data[3] = 0;
    out_matrix.data[4] = x_axis.y;
    out_matrix.data[5] = y_axis.y;
    out_matrix.data[6] = -z_axis.y;
    out_matrix.data[7] = 0;
    out_matrix.data[8] = x_axis.z;
    out_matrix.data[9] = y_axis.z;
    out_matrix.data[10] = -z_axis.z;
    out_matrix.data[11] = 0;
    out_matrix.data[12] = -vec3_dot(x_axis, position);
    out_matrix.data[13] = -vec3_dot(y_axis, position);
    out_matrix.data[14] = vec3_dot(z_axis, position);
    out_matrix.data[15] = 1.0f;

    return out_matrix;
}

INLINE Mat4 mat4_inverse(Mat4 matrix) {
    const f32* m = matrix.data;

    f32 t0 = m[10] * m[15];
    f32 t1 = m[14] * m[11];
    f32 t2 = m[6] * m[15];
    f32 t3 = m[14] * m[7];
    f32 t4 = m[6] * m[11];
    f32 t5 = m[10] * m[7];
    f32 t6 = m[2] * m[15];
    f32 t7 = m[14] * m[3];
    f32 t8 = m[2] * m[11];
    f32 t9 = m[10] * m[3];
    f32 t10 = m[2] * m[7];
    f32 t11 = m[6] * m[3];
    f32 t12 = m[8] * m[13];
    f32 t13 = m[12] * m[9];
    f32 t14 = m[4] * m[13];
    f32 t15 = m[12] * m[5];
    f32 t16 = m[4] * m[9];
    f32 t17 = m[8] * m[5];
    f32 t18 = m[0] * m[13];
    f32 t19 = m[12] * m[1];
    f32 t20 = m[0] * m[9];
    f32 t21 = m[8] * m[1];
    f32 t22 = m[0] * m[5];
    f32 t23 = m[4] * m[1];

    Mat4 out_matrix;
    f32* o = out_matrix.data;

    o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] + t2 * m[9] + t5 * m[13]);
    o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
    o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
    o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

    f32 d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

    o[0] = d * o[0];
    o[1] = d * o[1];
    o[2] = d * o[2];
    o[3] = d * o[3];
    o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
    o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
    o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
    o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
    o[8] =
        d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
    o[9] =
        d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
    o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
    o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
    o[12] =
        d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
    o[13] =
        d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
    o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
    o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

    return out_matrix;
}

#endif
