// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <bloblist.h>
#include <bootstage.h>
#include <command.h>
#include <display_options.h>
#include <lmb.h>
#include <malloc.h>
#include <mapmem.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

void __weak arch_dump_mem_attrs(void)
{
}

static void print_region(const char *name, ulong base, ulong size, ulong *uptop)
{
	ulong end = base + size;

	printf("%-12s %8lx %8lx %8lx", name, base, size, end);
	if (*uptop)
		printf(" %8lx", *uptop - end);
	putc('\n');
	*uptop = base;
}

static void show_lmb(const struct lmb *lmb, ulong *uptop)
{
	int i;

	for (i = lmb->used_mem.count - 1; i >= 0; i--) {
		const struct lmb_region *rgn = alist_get(&lmb->used_mem, i,
							 struct lmb_region);

		/*
		 * Assume that the top lmb region is the U-Boot region, so just
		 * take account of the memory not already reported
		 */
		if (lmb->used_mem.count - 1)
			print_region("lmb", rgn->base, *uptop - rgn->base,
				     uptop);
		else
			print_region("lmb", rgn->base, rgn->size, uptop);
		*uptop = rgn->base;
	}
}

static int do_meminfo(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	ulong upto, stk_bot;

	puts("DRAM:  ");
	print_size(gd->ram_size, "\n");

	if (!IS_ENABLED(CONFIG_CMD_MEMINFO_MAP))
		return 0;

	arch_dump_mem_attrs();

	printf("\n%-12s %8s %8s %8s %8s\n", "Region", "Base", "Size", "End",
	       "Gap");
	printf("------------------------------------------------\n");
	upto = 0;
	if (IS_ENABLED(CONFIG_VIDEO))
		print_region("video", gd_video_bottom(),
			     gd_video_size(), &upto);
	if (IS_ENABLED(CONFIG_TRACE))
		print_region("trace", map_to_sysmem(gd_trace_buff()),
			     gd_trace_size(), &upto);
	print_region("code", gd->relocaddr, gd->mon_len, &upto);
	print_region("malloc", map_to_sysmem((void *)mem_malloc_start),
		     mem_malloc_end - mem_malloc_start, &upto);
	print_region("board_info", map_to_sysmem(gd->bd),
		     sizeof(struct bd_info), &upto);
	print_region("global_data", map_to_sysmem((void *)gd),
		     sizeof(struct global_data), &upto);
	print_region("devicetree", map_to_sysmem(gd->fdt_blob),
		     fdt_totalsize(gd->fdt_blob), &upto);
	if (IS_ENABLED(CONFIG_BOOTSTAGE))
		print_region("bootstage", map_to_sysmem(gd_bootstage()),
			     bootstage_get_size(false), &upto);
	if (IS_ENABLED(CONFIG_BLOBLIST))
		print_region("bloblist", map_to_sysmem(gd_bloblist()),
			     bloblist_get_total_size(), &upto);
	stk_bot = gd->start_addr_sp - CONFIG_STACK_SIZE;
	print_region("stack", stk_bot, CONFIG_STACK_SIZE, &upto);
	if (IS_ENABLED(CONFIG_LMB))
		show_lmb(lmb_get(), &upto);
	print_region("free", gd->ram_base, upto - gd->ram_base, &upto);

	return 0;
}

U_BOOT_CMD(
	meminfo,	1,	1,	do_meminfo,
	"display memory information",
	""
);
