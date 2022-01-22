#define _GNU_SOURCE
#include "backends.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

static struct {
	void (**reports)(const char *, const struct cover_case_report *);
	size_t count;
} state;

static void *
backend_symbol(const char *backend, size_t backendlen, const char *suffix, size_t suffixlen) {
	static const char prefix[] = "cover_backend_";
	char symbol[sizeof (prefix) + backendlen + suffixlen], *error;
	void *func;

	memcpy(symbol, prefix, sizeof (prefix) - 1);
	memcpy(symbol + sizeof (prefix) - 1, backend, backendlen);
	memcpy(symbol + sizeof (prefix) - 1 + backendlen, suffix, suffixlen + 1);

	dlerror(); /* Clear any previous error */
	func = dlsym(RTLD_DEFAULT, symbol);
	error = dlerror();

	if (error != NULL) {
		fputs(error, stderr);
		fputc('\n', stderr);
	}

	return func;
}

void
backends_enable(const char *backend, const char *option) {
	static const char initsuffix[] = "_init", reportsuffix[] = "_report";
	const size_t backendlen = strlen(backend);
	void (*init)(const char *);
	void (*report)(const char *, const struct cover_case_report *);

	init = backend_symbol(backend, backendlen, initsuffix, sizeof (initsuffix) - 1);
	if (init != NULL) {
		init(option);
	}

	report = backend_symbol(backend, backendlen, reportsuffix, sizeof (reportsuffix) - 1);
	if (report != NULL) {
		state.reports = realloc(state.reports, sizeof (*state.reports) * ++(state.count));
		state.reports[state.count - 1] = report;
	}
}

void
backends_report(const char *name, const struct cover_case_report *report) {

	for (size_t i = 0; i < state.count; i++) {
		state.reports[i](name, report);
	}
}
