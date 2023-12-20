#include "collections/vector_tests.h"
#include "math/lineal_tests.h"
#include "test_runner.h"

int main(void) {

    test_runner_init();
    register_vec_tests();
    register_lineal_math_tests();
    test_runner_run_all_tests();
}
