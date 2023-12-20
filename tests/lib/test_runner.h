#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include "test.h"

typedef Test (*Test_FN)(void);

void test_runner_init(void);
void test_runner_register(Test_FN fn, const char* description);
void test_runner_run_all_tests(void);

#endif
