/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <vsprintf.h>

void dm_warn(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

void dm_dbg(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

int list_count_items(struct list_head *head)
{
	struct list_head *node;
	int count = 0;

	list_for_each(node, head)
		count++;

	return count;
}
