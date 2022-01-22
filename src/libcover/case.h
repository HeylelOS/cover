#ifndef CASE_H
#define CASE_H

#include <stdbool.h>
#include <signal.h>

struct cover_case_failure {
	const siginfo_t *siginfo;
	const char *message;
	const char *file;
	int lineno;
};

struct cover_case_report {
	struct cover_case_failure failure;
	bool failed;
};

/* CASE_H */
#endif
