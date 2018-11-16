// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <dm/ofnode.h>
#include <dm/util.h>
#include <linux/libfdt.h>
#include <vsprintf.h>

#ifdef CONFIG_DM_WARN
void dm_warn(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}
#endif

int list_count_items(struct list_head *head)
{
	struct list_head *node;
	int count = 0;

	list_for_each(node, head)
		count++;

	return count;
}

bool dm_fdt_pre_reloc(const void *blob, int offset)
{
	if (fdt_getprop(blob, offset, "u-boot,dm-pre-reloc", NULL))
		return true;

#ifdef CONFIG_TPL_BUILD
	if (fdt_getprop(blob, offset, "u-boot,dm-tpl", NULL))
		return true;
#elif defined(CONFIG_SPL_BUILD)
	if (fdt_getprop(blob, offset, "u-boot,dm-spl", NULL))
		return true;
#else
	/*
	 * In regular builds individual spl and tpl handling both
	 * count as handled pre-relocation for later second init.
	 */
	if (fdt_getprop(blob, offset, "u-boot,dm-spl", NULL) ||
	    fdt_getprop(blob, offset, "u-boot,dm-tpl", NULL))
		return true;
#endif

	return false;
}

bool dm_ofnode_pre_reloc(ofnode node)
{
	if (ofnode_read_bool(node, "u-boot,dm-pre-reloc"))
		return true;

#ifdef CONFIG_TPL_BUILD
	if (ofnode_read_bool(node, "u-boot,dm-tpl"))
		return true;
#elif defined(CONFIG_SPL_BUILD)
	if (ofnode_read_bool(node, "u-boot,dm-spl"))
		return true;
#else
	/*
	 * In regular builds individual spl and tpl handling both
	 * count as handled pre-relocation for later second init.
	 */
	if (ofnode_read_bool(node, "u-boot,dm-spl") ||
	    ofnode_read_bool(node, "u-boot,dm-tpl"))
		return true;
#endif

	return false;
}
