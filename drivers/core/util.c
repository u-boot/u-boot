/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <libfdt.h>
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

int dm_fdt_pre_reloc(const void *blob, int offset)
{
	if (fdt_getprop(blob, offset, "u-boot,dm-pre-reloc", NULL))
		return 1;

#ifdef CONFIG_TPL_BUILD
	if (fdt_getprop(blob, offset, "u-boot,dm-tpl", NULL))
		return 1;
#elif defined(CONFIG_SPL_BUILD)
	if (fdt_getprop(blob, offset, "u-boot,dm-spl", NULL))
		return 1;
#else
	/*
	 * In regular builds individual spl and tpl handling both
	 * count as handled pre-relocation for later second init.
	 */
	if (fdt_getprop(blob, offset, "u-boot,dm-spl", NULL) ||
	    fdt_getprop(blob, offset, "u-boot,dm-tpl", NULL))
		return 1;
#endif

	return 0;
}
