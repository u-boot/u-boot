/*
 * Simple unit test library for driver model
 *
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm/test.h>
#include <dm/ut.h>

struct dm_test_state;

void ut_fail(struct dm_test_state *dms, const char *fname, int line,
	     const char *func, const char *cond)
{
	printf("%s:%d, %s(): %s\n", fname, line, func, cond);
	dms->fail_count++;
}

void ut_failf(struct dm_test_state *dms, const char *fname, int line,
	      const char *func, const char *cond, const char *fmt, ...)
{
	va_list args;

	printf("%s:%d, %s(): %s: ", fname, line, func, cond);
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	putc('\n');
	dms->fail_count++;
}
