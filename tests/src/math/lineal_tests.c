#include "lineal_tests.h"
#include "test_runner.h"
#include <math/lineal.h>
#include <math/lineal_types.h>
#include <test.h>

Test vec2_constructors_test(void) {
    Vec2 origin = vec2_create(0.0, 0.0);
    Vec2 zero = vec2_zero();
    EXPECT_VEC_EQ(origin, zero);
    return OK;
}

Test vec2_add_test(void) {
    Vec2 a = vec2_create(1.0, 2.0);
    Vec2 b = vec2_create(0.5, -1.5);
    Vec2 add = vec2_add(a, b);
    Vec2 result_should_be = vec2_create(1.5, 0.5);
    EXPECT_VEC_EQ(add, result_should_be);
    return OK;
}

Test vec2_sub_test(void) {
    Vec2 a = vec2_create(1.0, 2.0);
    Vec2 b = vec2_create(0.5, -1.5);
    Vec2 sub = vec2_sub(a, b);
    Vec2 result = vec2_create(0.5, 3.5);
    EXPECT_VEC_EQ(sub, result);
    return OK;
}

// -------- Vec3 ------------//

Test vec3_constructor_test(void) {
    Vec3 zero = vec3_zero();
    Vec3 zero_result = vec3_create(0.0f, 0.0f, 0.0f);
    Vec3 one = vec3_one();
    Vec3 one_result = vec3_create(1.0f, 1.0f, 1.0f);
    EXPECT_VEC_EQ(zero, zero_result);
    EXPECT_VEC_EQ(one, one_result);
    return OK;
}

Test vec3_add_test(void) {
    Vec3 a = vec3_create(-1.0, 2.0, 5.0);
    Vec3 b = vec3_create(0.0, 1.0, 1.0);
    Vec3 expected = vec3_create(-1.0, 3.0, 6.0);
    Vec3 result = vec3_add(a, b);
    EXPECT_VEC_EQ(result, expected);
    return OK;
}

Test vec3_sub_test(void) {
    Vec3 a = vec3_create(-1.0, 2.0, 5.0);
    Vec3 b = vec3_create(0.0, 1.0, 1.0);
    Vec3 expected = vec3_create(-1.0, 1.0, 4.0);
    Vec3 result = vec3_sub(a, b);
    EXPECT_VEC_EQ(result, expected);
    return OK;
}

Test vec3_length_squared_test(void) {
    Vec3 p = vec3_create(5.0f, 2.0f, 2.0f);
    const f32 len = vec3_length_squared(p);
    f32 expected = 25.0 + 4.0 + 4.0;
    EXPECT_FLOAT_EQ(len, expected);
    return OK;
}

Test vec3_length_test(void) {
    Vec3 vec_test = vec3_create(2.0f, 1.5f, 0.5f);
    f32 expected_length = 2.55;
    EXPECT_FLOAT_EQ(vec3_length(vec_test), expected_length);
    return OK;
}

Test vec3_normalize_test(void) {
    Vec3 pos = vec3_create(0.4, 88.1, 999.2);
    Vec3 dir = vec3_normalized(pos);
    f32 magn = vec3_length(dir);
    EXPECT_FLOAT_EQ(magn, 1.0f);
    return OK;
}

void register_lineal_math_tests(void) {
    // vec2
    test_runner_register(vec2_constructors_test, "Vector2 constructors test");
    test_runner_register(vec2_add_test, "Vector2 addition test");
    test_runner_register(vec2_sub_test, "Vector2 substraction test");
    // vec3
    test_runner_register(vec3_constructor_test, "Vec3 constructors test");
    test_runner_register(vec3_add_test, "Vec3 addition test");
    test_runner_register(vec3_sub_test, "Vec3 subtraction test");
    test_runner_register(vec3_length_squared_test, "Vec3 length squared test");
    test_runner_register(vec3_length_test, "Vec3 magnitute test");
    test_runner_register(vec3_normalize_test, "Vec3 normalization test");
}
