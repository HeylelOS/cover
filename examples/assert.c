#include <cover/suite.h>

#include <stdbool.h>

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

