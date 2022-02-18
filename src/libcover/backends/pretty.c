#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#include "../case.h"

#define ANSI_ESC "\x1B"
#define ANSI_CSI ANSI_ESC "["
#define ANSI_SGR(n) ANSI_CSI n "m"

#define ANSI_SGR_RESET ANSI_SGR("0")
#define ANSI_SGR_BOLD(color) ANSI_SGR("1;" color)

#define ANSI_COLOR_BLACK   "30"
#define ANSI_COLOR_RED     "31"
#define ANSI_COLOR_GREEN   "32"
#define ANSI_COLOR_YELLOW  "33"
#define ANSI_COLOR_BLUE    "34"
#define ANSI_COLOR_MAGENTA "35"
#define ANSI_COLOR_CYAN    "36"
#define ANSI_COLOR_WHITE   "37"

struct si_code_description {
	int code;
	const char *reason;
};

static FILE *output;
static bool colorized;
static unsigned int failures, total;

static void
pretty_info(const char *format, ...) {
	va_list ap;

	if (colorized) {
		fputs(ANSI_SGR_BOLD(ANSI_COLOR_WHITE), output);
	}

	va_start(ap, format);
	vfprintf(output, format, ap);
	va_end(ap);

	if (colorized) {
		fputs(ANSI_SGR_RESET, output);
	}
}

static void
pretty_pass(const char *format, ...) {
	va_list ap;

	if (colorized) {
		fputs("["ANSI_SGR_BOLD(ANSI_COLOR_GREEN)"PASS"ANSI_SGR_RESET"] ", output);
	} else {
		fputs("[PASS] ", output);
	}

	va_start(ap, format);
	vfprintf(output, format, ap);
	va_end(ap);
}

static void
pretty_fail(const char *format, ...) {
	va_list ap;

	if (colorized) {
		fputs("["ANSI_SGR_BOLD(ANSI_COLOR_RED)"FAIL"ANSI_SGR_RESET"] ", output);
	} else {
		fputs("[FAIL] ", output);
	}

	va_start(ap, format);
	vfprintf(output, format, ap);
	va_end(ap);
}

static void
pretty_diagnosis(const char *format, ...) {
	va_list ap;

	if (colorized) {
		fputs(ANSI_SGR_BOLD(ANSI_COLOR_YELLOW), output);
	}
	fputs("       > ", output);

	va_start(ap, format);
	vfprintf(output, format, ap);
	va_end(ap);

	if (colorized) {
		fputs(ANSI_SGR_RESET, output);
	}
}

static void
pretty_diagnose_signal_code_reason(int code, const struct si_code_description *descriptions, size_t descriptionscount) {
	size_t i = 0;

	while (i < descriptionscount && descriptions[i].code != code) {
		i++;
	}

	if (i != descriptionscount) {
		pretty_diagnosis("%s\n", descriptions[i].reason);
	}
}

static void
pretty_diagnose_signal(const siginfo_t *info) {
	static const struct si_code_description bus_descriptions[] = {
		{ .code = BUS_ADRALN, .reason = "Invalid address alignment." },
		{ .code = BUS_ADRERR, .reason = "Nonexistent physical address." },
		{ .code = BUS_OBJERR, .reason = "Object-specific hardware error." },
	};
	static const struct si_code_description fpe_descriptions[] = {
		{ .code = FPE_INTDIV, .reason = "Integer divide by zero." },
		{ .code = FPE_INTOVF, .reason = "Integer overflow." },
		{ .code = FPE_FLTDIV, .reason = "Floating-point divide by zero." },
		{ .code = FPE_FLTOVF, .reason = "Floating-point overflow." },
		{ .code = FPE_FLTUND, .reason = "Floating-point underflow." },
		{ .code = FPE_FLTRES, .reason = "Floating-point inexact result." },
		{ .code = FPE_FLTINV, .reason = "Invalid floating-point operation." },
		{ .code = FPE_FLTSUB, .reason = "Subscript out of range." },
	};
	static const struct si_code_description ill_descriptions[] = {
		{ .code = ILL_ILLOPC, .reason = "Illegal opcode." },
		{ .code = ILL_ILLOPN, .reason = "Illegal operand." },
		{ .code = ILL_ILLADR, .reason = "Illegal addressing mode." },
		{ .code = ILL_ILLTRP, .reason = "Illegal trap." },
		{ .code = ILL_PRVOPC, .reason = "Privileged opcode." },
		{ .code = ILL_PRVREG, .reason = "Privileged register." },
		{ .code = ILL_COPROC, .reason = "Coprocessor error." },
		{ .code = ILL_BADSTK, .reason = "Internal stack error." },
	};
	static const struct si_code_description segv_descriptions[] = {
		{ .code = SEGV_MAPERR, .reason = "Address not mapped to object." },
		{ .code = SEGV_ACCERR, .reason = "Invalid permissions for mapped object." },
	};

	pretty_diagnosis("Received signal %d (%s)\n", info->si_signo, strsignal(info->si_signo));
	switch (info->si_signo) {
	case SIGABRT:
		pretty_diagnosis("Process abort signal.\n");
		break;
	case SIGBUS:
		pretty_diagnosis("Access to an undefined portion of a memory object at %p.\n", info->si_addr);
		pretty_diagnose_signal_code_reason(info->si_code, bus_descriptions, sizeof (bus_descriptions) / sizeof (*bus_descriptions));
		break;
	case SIGFPE:
		pretty_diagnosis("Erroneous arithmetic operation at %p.\n", info->si_addr);
		pretty_diagnose_signal_code_reason(info->si_code, fpe_descriptions, sizeof (fpe_descriptions) / sizeof (*fpe_descriptions));
		break;
	case SIGHUP:
		pretty_diagnosis("Hangup.\n");
		break;
	case SIGILL:
		pretty_diagnosis("Illegal instruction at %p.\n", info->si_addr);
		pretty_diagnose_signal_code_reason(info->si_code, ill_descriptions, sizeof (ill_descriptions) / sizeof (*ill_descriptions));
		break;
	case SIGPIPE:
		pretty_diagnosis("Write on a pipe with no one to read it.\n");
		break;
	case SIGSEGV:
		pretty_diagnosis("Invalid memory reference at %p.\n", info->si_addr);
		pretty_diagnose_signal_code_reason(info->si_code, segv_descriptions, sizeof (segv_descriptions) / sizeof (*segv_descriptions));
		break;
	case SIGTRAP:
		pretty_diagnosis("Trace/breakpoint trap.\n");
		break;
	}
}

static void
cover_backend_pretty_fini(void) {

	pretty_info("End of test suite, %u failure%s out of %u test%s\n", failures, failures != 1 ? "s" : "", total, total != 1 ? "s" : "");

	if (output != stdout) {
		fclose(output);
	}
}

void
cover_backend_pretty_init(const char *filename) {

	if (strcmp(filename, "-") != 0) {
		output = fopen(filename, "w");
		if (output == NULL) {
			err(EXIT_FAILURE, "fopen %s", filename);
		}
	} else {
		output = stdout;
	}

	colorized = !!isatty(fileno(output));

	pretty_info("Starting test suite\n");

	atexit(cover_backend_pretty_fini);
}

void
cover_backend_pretty_report(const char *name, const struct cover_case_report *report) {

	if (report->failed) {

		if (report->failure.file != NULL) {
			pretty_fail("%s in file '%s' at line %d: %s\n", name, report->failure.file, report->failure.lineno, report->failure.message);
		} else {
			pretty_fail("%s: %s\n", name, report->failure.message);
		}

		if (report->failure.siginfo != NULL) {
			pretty_diagnose_signal(report->failure.siginfo);
		}

		failures++;
	} else {
		pretty_pass("%s\n", name);
	}

	total++;
}
