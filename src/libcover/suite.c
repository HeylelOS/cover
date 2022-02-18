#include <cover/suite.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <err.h>

#include "backends.h"
#include "case.h"

static void
cover_err_sigaction(int sig, siginfo_t *info, void *ucontext);

static void
cover_term_handler(int sig);

static struct {
	struct cover_case_report report;
	sigjmp_buf env;
	bool terminate;
} state;

static struct {
	const int signo;
	const struct sigaction act;
	struct sigaction oldact;
} sigstates[] = {
	{ .signo = SIGABRT, .act = { .sa_sigaction = cover_err_sigaction, .sa_flags = SA_SIGINFO } },
	{ .signo = SIGBUS,  .act = { .sa_sigaction = cover_err_sigaction, .sa_flags = SA_SIGINFO } },
	{ .signo = SIGFPE,  .act = { .sa_sigaction = cover_err_sigaction, .sa_flags = SA_SIGINFO } },
	{ .signo = SIGHUP,  .act = { .sa_sigaction = cover_err_sigaction, .sa_flags = SA_SIGINFO } },
	{ .signo = SIGILL,  .act = { .sa_sigaction = cover_err_sigaction, .sa_flags = SA_SIGINFO } },
	{ .signo = SIGPIPE, .act = { .sa_sigaction = cover_err_sigaction, .sa_flags = SA_SIGINFO } },
	{ .signo = SIGSEGV, .act = { .sa_sigaction = cover_err_sigaction, .sa_flags = SA_SIGINFO } },
	{ .signo = SIGTRAP, .act = { .sa_sigaction = cover_err_sigaction, .sa_flags = SA_SIGINFO } },

	{ .signo = SIGINT,  .act = { .sa_handler = cover_term_handler } },
	{ .signo = SIGQUIT, .act = { .sa_handler = cover_term_handler } },
	{ .signo = SIGTERM, .act = { .sa_handler = cover_term_handler } },
	{ .signo = SIGXCPU, .act = { .sa_handler = cover_term_handler } },
	{ .signo = SIGXFSZ, .act = { .sa_handler = cover_term_handler } },
};

void
cover_fail_at(const char *message, const char *file, int lineno) {
	const char * const separator = strrchr(file, '/');

	state.report.failed = true;
	state.report.failure.message = message;
	state.report.failure.file = separator != NULL ? separator + 1 : file;
	state.report.failure.lineno = lineno;

	siglongjmp(state.env, 1);
}

static inline bool
is_dash_or_dash_dash(const char *string) {
	return *string == '-' && (*++string == '\0' || (*string == '-' && *++string == '\0'));
}

static void
cover_setup_backends(int argc, char **argv) {

	while (optind < argc && !is_dash_or_dash_dash(argv[optind]) && *argv[optind] == '-') {
		const char * const backend = argv[optind++] + 1;

		if (optind == argc) {
			warnx("option requires an argument -- -%s", backend);
			break;
		}

		backends_enable(backend, argv[optind++]);
	}
}

static void
cover_err_sigaction(int sig, siginfo_t *info, void *ucontext) {
	static siginfo_t siginfo;

	siginfo = *info;

	state.report.failed = true;
	state.report.failure.message = strsignal(sig);
	state.report.failure.siginfo = &siginfo;

	siglongjmp(state.env, 1);
}

static void
cover_term_handler(int sig) {
	state.terminate = true;
}

static void
cover_setup_sigactions(void) {
	for (unsigned int i = 0; i < sizeof (sigstates) / sizeof (*sigstates); i++) {
		sigaction(sigstates[i].signo, &sigstates[i].act, &sigstates[i].oldact);
	}
}

static void
cover_restore_sigactions(void) {
	for (unsigned int i = 0; i < sizeof (sigstates) / sizeof (*sigstates); i++) {
		sigaction(sigstates[i].signo, &sigstates[i].oldact, NULL);
	}
}

static void
cover_run_suite(void) {
	extern const struct cover_case cover_suite[];
	const struct cover_case *suitecase = cover_suite;

	while (suitecase->type != COVER_CASE_TYPE_END && !state.terminate) {

		memset(&state.report, 0, sizeof (state.report));

		if (sigsetjmp(state.env, 1) == 0) {
			suitecase->func();
		}

		backends_report(suitecase->name, &state.report);

		suitecase++;
	}
}

int
main(int argc, char **argv) {
	extern void __attribute__((weak)) cover_suite_init(int, char **);
	extern void __attribute__((weak)) cover_suite_fini(int *);
	int status = EXIT_SUCCESS;

	cover_setup_backends(argc, argv);

	if (cover_suite_init) {
		cover_suite_init(argc, argv);
	}

	cover_setup_sigactions();
	cover_run_suite();
	cover_restore_sigactions();

	if (cover_suite_fini) {
		cover_suite_fini(&status);
	}

	return status;
}

