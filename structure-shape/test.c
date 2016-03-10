#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "struct.h"

/* invariant valid for entry structure */
#define entry_assert(e) \
	assert_true(e.timer_soft_sec < e.timer_hard_sec); \
	assert_true(e.timer_soft_kb < e.timer_hard_kb); \
	assert_true(e.timer_hard_sec - e.timer_soft_sec >= MINDIFF); \
	assert_true(e.timer_hard_kb - e.timer_soft_kb >= MINDIFF);

#define LENGTH(x) (sizeof x / sizeof x[0])

/* tests */

// a test case that does nothing and succeeds (tests framework)
static void null_test_success(void **state) {
    (void) state; /* unused */
}

// test default value handling
static void entry_default(void **state) {
	char *argv[] = { "shape",  "0", "0", "0", "0"};

	entry_t e = shape(LENGTH(argv), argv);
	entry_assert(e);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(null_test_success),
        cmocka_unit_test(entry_default),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}