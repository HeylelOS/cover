#include <cover/suite.h>

#include <stdlib.h>

static void
test_signal_sigabrt(void) {
	abort();
}

static void
test_signal_sigsegv(void) {
	int * const ptr = NULL;
	*ptr = 451;
}

const struct cover_case cover_suite[] = {
	COVER_SUITE_TEST(test_signal_sigabrt),
	COVER_SUITE_TEST(test_signal_sigsegv),
	COVER_SUITE_END,
};

