#ifndef BACKENDS_H
#define BACKENDS_H

struct cover_case_report;

void
backends_enable(const char *backend, const char *option);

void
backends_report(const char *name, const struct cover_case_report *report);

/* BACKENDS_H */
#endif
