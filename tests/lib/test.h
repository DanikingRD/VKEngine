#ifndef TEST_H
#define TEST_H

#include <core/log.h>

typedef enum TestResult {
    TEST_RESULT_OK,
    TEST_RESULT_FAIL,
    TEST_RESULT_IGNORE,
} TestResult;

#define Test TestResult

#define OK TEST_RESULT_OK
#define FAIL TEST_RESULT_FAIL
#define IGNORE TEST_RESULT_IGNORE

#define EXPECT_EQ(left, right)                                                                     \
    if (left != right) {                                                                           \
        ERROR("assertion `left == right` failed.\n left: %d\nright: %d", left, right);             \
        return FAIL;                                                                               \
    }

#define EXPECT_NEQ(left, right)                                                                    \
    if (left == right) {                                                                           \
        ERROR("assertion `left != right` failed.\n \"left\": %d\n\"right\": %d", left, right);     \
        return FAIL;                                                                               \
    }
#define EXPECT_FLOAT_EQ(left, right)                                                               \
    if (fabs(left - right) > 0.001f) {                                                             \
        ERROR("assertion `left == right` failed.\n left: %.3f\nright: %.3f", left, right);         \
        return FAIL;                                                                               \
    }

#define EXPECT_VEC_EQ(v1, v2)                                                                      \
    do {                                                                                           \
        for (u64 i = 0; i < sizeof((v1).data) / sizeof((v1).data[0]); ++i) {                       \
            EXPECT_FLOAT_EQ((v1).data[i], (v2).data[i]);                                           \
        }                                                                                          \
    } while (0)

#endif
