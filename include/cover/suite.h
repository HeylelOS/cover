#ifndef COVER_SUITE_H
#define COVER_SUITE_H

#define COVER_SUITE_TEST(func)         { COVER_CASE_TYPE_TEST, 0, func, #func, }
#define COVER_SUITE_END                { COVER_CASE_TYPE_END, }

enum cover_case_type {
	COVER_CASE_TYPE_END,
	COVER_CASE_TYPE_TEST,
};

struct cover_case {
	enum cover_case_type type;
	unsigned int flags;
	void (*func)(void);
	const char *name;
};

#define cover_assert(condition, message) (condition) ? (void)0 : cover_fail("'" #condition "': " message)

#define cover_fail(message) cover_fail_at(message, __FILE__, __LINE__)

void
cover_fail_at(const char *message, const char *file, int lineno);

/* COVER_SUITE_H */
#endif
