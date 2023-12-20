#include <collections/vector.h>
#include <collections/vector_tests.h>
#include <core/str.h>
#include <test.h>
#include <test_runner.h>

Test vector_new_test(void) {
    u64 cap = 12;
    Vector(i32) test = _vector_new(cap, sizeof(i32));
    EXPECT_EQ(vector_length(test), 0);
    EXPECT_EQ(vector_capacity(test), cap);
    EXPECT_EQ(vector_stride(test), sizeof(i32));
    vector_free(test);
    return OK;
}

Test vector_free_test(void) {
    Vector(i32) vec = vector_new(i32);
    EXPECT_NEQ(vec, 0);
    vector_free(vec);
    EXPECT_EQ(vec, 0);
    return OK;
}

Test vector_push_test(void) {
    Vector(i32) vec = vector_new(i32);
    vector_push(vec, 5);
    vector_push(vec, 10);
    i32 first = vec[0];
    i32 second = vec[1];
    EXPECT_EQ(first, 5);
    EXPECT_EQ(second, 10);
    EXPECT_EQ(vector_length(vec), 2);
    vector_free(vec);
    return OK;
}

Test vec_pop_test(void) {
    Vector(const char*) vec = vector_new(const char*);
    vector_push(vec, &"hello world!");
    vector_push(vec, &"123456789");

    const char* pop_first;
    vector_pop(vec, &pop_first);

    const char* pop_second;
    vector_pop(vec, &pop_second);

    EXPECT_EQ(str_equals(pop_first, "123456789"), true);
    EXPECT_EQ(str_equals(pop_second, "hello world!"), true);

    vector_free(vec);

    return OK;
}

Test vec_remove_test(void) {
    Vector(i32) vec = vector_new(i32);

    for (i32 i = 0; i < 3; i++) {
        vector_push(vec, i);
    }
    i32 first, second, third;
    vector_remove(vec, 0, &first);
    vector_remove(vec, 0, &second);
    vector_remove(vec, 0, &third);

    EXPECT_EQ(first, 0);
    EXPECT_EQ(second, 1);
    EXPECT_EQ(third, 2);
    return OK;
}

void register_vec_tests(void) {
    test_runner_register(vector_new_test, "A new vector is allocated");
    test_runner_register(vector_free_test, "Vector is cleaned up");
    test_runner_register(vector_push_test, "Element gets pushed back");
    test_runner_register(vec_pop_test, "Last element gets popped");
    test_runner_register(vec_remove_test, "Element at index N is removed.");
}
