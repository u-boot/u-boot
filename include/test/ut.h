/*
 * Simple unit test library
 *
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __TEST_UT_H
#define __TEST_UT_H

struct unit_test_state;

/**
 * ut_fail() - Record failure of a unit test
 *
 * @uts: Test state
 * @fname: Filename where the error occured
 * @line: Line number where the error occured
 * @func: Function name where the error occured
 * @cond: The condition that failed
 */
void ut_fail(struct unit_test_state *uts, const char *fname, int line,
	     const char *func, const char *cond);

/**
 * ut_failf() - Record failure of a unit test
 *
 * @uts: Test state
 * @fname: Filename where the error occured
 * @line: Line number where the error occured
 * @func: Function name where the error occured
 * @cond: The condition that failed
 * @fmt: printf() format string for the error, followed by args
 */
void ut_failf(struct unit_test_state *uts, const char *fname, int line,
	      const char *func, const char *cond, const char *fmt, ...)
			__attribute__ ((format (__printf__, 6, 7)));


/* Assert that a condition is non-zero */
#define ut_assert(cond)							\
	if (!(cond)) {							\
		ut_fail(uts, __FILE__, __LINE__, __func__, #cond);	\
		return CMD_RET_FAILURE;					\
	}

/* Assert that a condition is non-zero, with printf() string */
#define ut_assertf(cond, fmt, args...)					\
	if (!(cond)) {							\
		ut_failf(uts, __FILE__, __LINE__, __func__, #cond,	\
			 fmt, ##args);					\
		return CMD_RET_FAILURE;					\
	}

/* Assert that two int expressions are equal */
#define ut_asserteq(expr1, expr2) {					\
	unsigned int val1 = (expr1), val2 = (expr2);			\
									\
	if (val1 != val2) {						\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr1 " == " #expr2,				\
			 "Expected %d, got %d", val1, val2);		\
		return CMD_RET_FAILURE;					\
	}								\
}

/* Assert that two string expressions are equal */
#define ut_asserteq_str(expr1, expr2) {					\
	const char *val1 = (expr1), *val2 = (expr2);			\
									\
	if (strcmp(val1, val2)) {					\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr1 " = " #expr2,				\
			 "Expected \"%s\", got \"%s\"", val1, val2);	\
		return CMD_RET_FAILURE;					\
	}								\
}

/* Assert that two pointers are equal */
#define ut_asserteq_ptr(expr1, expr2) {					\
	const void *val1 = (expr1), *val2 = (expr2);			\
									\
	if (val1 != val2) {						\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr1 " = " #expr2,				\
			 "Expected %p, got %p", val1, val2);		\
		return CMD_RET_FAILURE;					\
	}								\
}

/* Assert that a pointer is not NULL */
#define ut_assertnonnull(expr) {					\
	const void *val = (expr);					\
									\
	if (val == NULL) {						\
		ut_failf(uts, __FILE__, __LINE__, __func__,		\
			 #expr " = NULL",				\
			 "Expected non-null, got NULL");		\
		return CMD_RET_FAILURE;					\
	}								\
}

/* Assert that an operation succeeds (returns 0) */
#define ut_assertok(cond)	ut_asserteq(0, cond)

#endif
