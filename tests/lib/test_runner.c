#include "test_runner.h"
#include "core/log.h"
#include "types.h"
#include <collections/vector.h>
#include <core/instant.h>

typedef struct TestEntry {
    Test_FN fn;
    const char* desc;
} TestEntry;

Vector(TestEntry) tests;

void test_runner_init(void) {
    tests = vector_new(TestEntry);
    INFO("Test runner initialized.");
}

void test_runner_register(Test_FN fn, const char* description) {
    TestEntry entry = {
        .fn = fn,
        .desc = description,
    };
    vector_push(tests, entry);
}

void test_runner_run_all_tests(void) {
    u32 passed = 0;
    u32 skipped = 0;
    u32 failed = 0;
    u32 test_count = vector_length(tests);
    INFO("Running %d tests", test_count);
    Instant test_runner_begin;
    instant_now(&test_runner_begin);

    for (u32 i = 0; i < test_count; i++) {
        TestEntry* entry = &tests[i];

        Instant test_begin;
        instant_now(&test_begin);
        Test result = entry->fn();
        f64 test_duration = instant_elapsed(&test_runner_begin);

        switch (result) {
        case TEST_RESULT_OK:
            INFO("test %s -> OK in %.3fs", entry->desc, test_duration);
            passed++;
            break;
        case TEST_RESULT_FAIL:
            ERROR("test %s -> FAILED in %.3fs", entry->desc, test_duration);
            failed++;
            break;

        case TEST_RESULT_IGNORE:
            WARN("test %s -> IGNORE in %.3fs", entry->desc, test_duration);
            skipped++;
            break;
        }
    }
    f64 execution_time = instant_elapsed(&test_runner_begin);
    INFO("test result: %d passed; %d failed; %d skipped; finished in %.3fs", passed, failed,
         skipped, execution_time);

    vector_free(tests);
}
