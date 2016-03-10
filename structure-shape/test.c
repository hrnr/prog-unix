#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include "struct.h"

/* helpers */

/* invariant valid for entry structure */
#define entry_assert(e) \
	assert_true(e.timer_soft_sec < e.timer_hard_sec); \
	assert_true(e.timer_soft_kb < e.timer_hard_kb); \
	assert_true(e.timer_hard_sec - e.timer_soft_sec >= MINDIFF); \
	assert_true(e.timer_hard_kb - e.timer_soft_kb >= MINDIFF);

#define LENGTH(x) (sizeof x / sizeof x[0])

struct arg {
	char v[30];
};
// args must be defined in context
#define ARG(index) (args[index].v)

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

// monkey testing
static void entry_random(void **state) {
	const size_t iterations = 300;
	struct arg args[4];
	char *argv[5] = {"shape", ARG(0), ARG(1), ARG(2), ARG(3)};
	// choosen by fair dice roll
	unsigned int seed = 0xdeadbeef;

	for(size_t i = 0; i < iterations; ++i) {
		// generate random arguments
		for (int i = 0; i < 4; ++i) {
			snprintf(ARG(i), 30, "%d", rand_r(&seed));
		}
		entry_t e = shape(LENGTH(argv), argv);
		entry_assert(e);
	}
}

static void entry_overflow(void **state) {
	struct arg args[4];
	char *argv[5] = {"shape", ARG(0), ARG(1), ARG(2), ARG(3)};

	/* max */
	for (int i = 0; i < 4; ++i) {
		snprintf(ARG(i), 30, "%d", UINTMAX_MAX);
	}
	entry_t e = shape(LENGTH(argv), argv);
	entry_assert(e);

	/* min */
	for (int i = 0; i < 4; ++i) {
		snprintf(ARG(i), 30, "%d", INTMAX_MIN);
	}
	e = shape(LENGTH(argv), argv);
	entry_assert(e);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(null_test_success),
        cmocka_unit_test(entry_default),
        cmocka_unit_test(entry_random),
        cmocka_unit_test(entry_overflow),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}