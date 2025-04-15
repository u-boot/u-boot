// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#include <bootm.h>
#include <cpu_func.h>
#include <env.h>
#include <image.h>
#include <irq_func.h>
#include <log.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define NIOS_MAGIC 0x534f494e /* enable command line and initrd passing */

int do_bootm_linux(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;
	void (*kernel)(int, int, int, char *) = (void *)images->ep;
	char *commandline = env_get("bootargs");
	ulong initrd_start = images->rd_start;
	ulong initrd_end = images->rd_end;
	char *of_flat_tree = NULL;
#if defined(CONFIG_OF_LIBFDT)
	/* did generic code already find a device tree? */
	if (images->ft_len)
		of_flat_tree = images->ft_addr;
#endif
	/* TODO: Clean this up - the DT should already be set up */
	if (!of_flat_tree && bmi->argc > 1)
		of_flat_tree = (char *)hextoul(bmi->argv[1], NULL);
	if (of_flat_tree)
		initrd_end = (ulong)of_flat_tree;

	/*
	 * allow the PREP bootm subcommand, it is required for bootm to work
	 */
	if (flag & BOOTM_STATE_OS_PREP)
		return 0;

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	/* flushes data and instruction caches before calling the kernel */
	disable_interrupts();
	flush_dcache_all();

	debug("bootargs=%s @ 0x%lx\n", commandline, (ulong)&commandline);
	debug("initrd=0x%lx-0x%lx\n", (ulong)initrd_start, (ulong)initrd_end);
	/* kernel parameters passing
	 * r4 : NIOS magic
	 * r5 : initrd start
	 * r6 : initrd end or fdt
	 * r7 : kernel command line
	 * fdt is passed to kernel via r6, the same as initrd_end. fdt will be
	 * verified with fdt magic. when both initrd and fdt are used at the
	 * same time, fdt must follow immediately after initrd.
	 */
	kernel(NIOS_MAGIC, initrd_start, initrd_end, commandline);
	/* does not return */

	return 1;
}
