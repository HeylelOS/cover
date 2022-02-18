#include <cover/suite.h>

#include <stdio.h>
#include <stdbool.h>

void
cover_suite_init(int argc, char **argv) {
	fputs("Suite runtime initialization example\n", stderr);
}

void
cover_suite_fini(int *statusp) {
	fputs("Suite runtime finalization example\n", stderr);
}

static void
test_assert_success(void) {
	cover_assert(true, "Always valid assertion");
}

static void
test_assert_failure(void) {
	cover_assert(false, "Always invalid assertion");
}

const struct cover_case cover_suite[] = {
	COVER_SUITE_TEST(test_assert_success),
	COVER_SUITE_TEST(test_assert_failure),
	COVER_SUITE_END,
};

