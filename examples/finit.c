#include <cover/suite.h>

#include <stdio.h>

void
cover_suite_init(int argc, char **argv) {
	puts("Suite runtime initialization example");
}

void
cover_suite_fini(void) {
	puts("Suite runtime finalization example");
}

const struct cover_case cover_suite[] = {
	COVER_SUITE_END,
};

