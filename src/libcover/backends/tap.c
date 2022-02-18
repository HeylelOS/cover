#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#include "../case.h"

struct si_code_description {
	int code;
	const char *reason;
};

static FILE *output;
static unsigned int total;

static void
tap_diagnostic_signal_code(int code, const struct si_code_description *descriptions, size_t descriptionscount) {
	size_t i = 0;

	while (i < descriptionscount && descriptions[i].code != code) {
		i++;
	}

	if (i != descriptionscount) {
		fprintf(output, "# %s\n", descriptions[i].reason);
	}
}

static void
tap_diagnostic_signal(const siginfo_t *info) {
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

	fprintf(output, "# Received signal %d (%s)\n", info->si_signo, strsignal(info->si_signo));
	switch (info->si_signo) {
	case SIGABRT:
		fputs("# Process abort signal.\n", output);
		break;
	case SIGBUS:
		fprintf(output, "# Access to an undefined portion of a memory object at %p.\n", info->si_addr);
		tap_diagnostic_signal_code(info->si_code, bus_descriptions, sizeof (bus_descriptions) / sizeof (*bus_descriptions));
		break;
	case SIGFPE:
		fprintf(output, "# Erroneous arithmetic operation at %p.\n", info->si_addr);
		tap_diagnostic_signal_code(info->si_code, fpe_descriptions, sizeof (fpe_descriptions) / sizeof (*fpe_descriptions));
		break;
	case SIGHUP:
		fputs("# Hangup.\n", output);
		break;
	case SIGILL:
		fprintf(output, "# Illegal instruction at %p.\n", info->si_addr);
		tap_diagnostic_signal_code(info->si_code, ill_descriptions, sizeof (ill_descriptions) / sizeof (*ill_descriptions));
		break;
	case SIGPIPE:
		fputs("# Write on a pipe with no one to read it.\n", output);
		break;
	case SIGSEGV:
		fprintf(output, "# Invalid memory reference at %p.\n", info->si_addr);
		tap_diagnostic_signal_code(info->si_code, segv_descriptions, sizeof (segv_descriptions) / sizeof (*segv_descriptions));
		break;
	case SIGTRAP:
		fputs("# Trace/breakpoint trap.\n", output);
		break;
	}
}

static void
cover_backend_tap_fini(void) {

	fprintf(output, "%u..%u\n", total != 0, total);

	if (output != stdout) {
		fclose(output);
	}
}

void
cover_backend_tap_init(const char *filename) {

	if (strcmp(filename, "-") != 0) {
		output = fopen(filename, "w");
		if (output == NULL) {
			err(EXIT_FAILURE, "fopen %s", filename);
		}
	} else {
		output = stdout;
	}

	atexit(cover_backend_tap_fini);
}

void
cover_backend_tap_report(const char *name, const struct cover_case_report *report) {

	total++;

	if (report->failed) {
		fputs("not ", output);
	}

	fprintf(output, "ok %d %s\n", total, name);

	if (report->failed) {

		if (report->failure.file != NULL) {
			fprintf(output, "# Failure in file '%s' at line %d: %s\n", report->failure.file, report->failure.lineno, report->failure.message);
		} else {
			fprintf(output, "# %s\n", report->failure.message);
		}

		if (report->failure.siginfo != NULL) {
			tap_diagnostic_signal(report->failure.siginfo);
		}
	}
}
