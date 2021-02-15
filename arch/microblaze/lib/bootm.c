// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 Michal Simek
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Michal  SIMEK <monstr@monstr.eu>
 * Yasushi SHOJI <yashi@atmark-techno.com>
 */

#include <common.h>
#include <bootstage.h>
#include <command.h>
#include <cpu_func.h>
#include <env.h>
#include <fdt_support.h>
#include <hang.h>
#include <image.h>
#include <lmb.h>
#include <log.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <u-boot/zlib.h>
#include <asm/byteorder.h>

DECLARE_GLOBAL_DATA_PTR;

static ulong get_sp(void)
{
	ulong ret;

	asm("addik %0, r1, 0" : "=r"(ret) : );
	return ret;
}

void arch_lmb_reserve(struct lmb *lmb)
{
	ulong sp, bank_end;
	int bank;

	/*
	 * Booting a (Linux) kernel image
	 *
	 * Allocate space for command line and board info - the
	 * address should be as high as possible within the reach of
	 * the kernel (see CONFIG_SYS_BOOTMAPSZ settings), but in unused
	 * memory, which means far enough below the current stack
	 * pointer.
	 */
	sp = get_sp();
	debug("## Current stack ends at 0x%08lx ", sp);

	/* adjust sp by 4K to be safe */
	sp -= 4096;
	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		if (sp < gd->bd->bi_dram[bank].start)
			continue;
		bank_end = gd->bd->bi_dram[bank].start +
			gd->bd->bi_dram[bank].size;
		if (sp >= bank_end)
			continue;
		lmb_reserve(lmb, sp, bank_end - sp);
		break;
	}
}

static void boot_jump_linux(bootm_headers_t *images, int flag)
{
	void (*thekernel)(char *cmdline, ulong rd, ulong dt);
	ulong dt = (ulong)images->ft_addr;
	ulong rd_start = images->initrd_start;
	ulong cmdline = images->cmdline_start;
	int fake = (flag & BOOTM_STATE_OS_FAKE_GO);

	thekernel = (void (*)(char *, ulong, ulong))images->ep;

	debug("## Transferring control to Linux (at address 0x%08lx) ",
	      (ulong)thekernel);
	debug("cmdline 0x%08lx, ramdisk 0x%08lx, FDT 0x%08lx...\n",
	      cmdline, rd_start, dt);
	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	printf("\nStarting kernel ...%s\n\n", fake ?
	       "(fake run for tracing)" : "");
	bootstage_mark_name(BOOTSTAGE_ID_BOOTM_HANDOFF, "start_kernel");

#ifdef XILINX_USE_DCACHE
	flush_cache(0, XILINX_DCACHE_BYTE_SIZE);
#endif

	if (!fake) {
		/*
		 * Linux Kernel Parameters (passing device tree):
		 * r5: pointer to command line
		 * r6: pointer to ramdisk
		 * r7: pointer to the fdt, followed by the board info data
		 */
		thekernel((char *)cmdline, rd_start, dt);
		/* does not return */
	}
}

static void boot_prep_linux(bootm_headers_t *images)
{
	if (IMAGE_ENABLE_OF_LIBFDT && images->ft_len) {
		debug("using: FDT\n");
		if (image_setup_linux(images)) {
			printf("FDT creation failed! hanging...");
			hang();
		}
	}
}

int do_bootm_linux(int flag, int argc, char *const argv[],
		   bootm_headers_t *images)
{
	images->cmdline_start = (ulong)env_get("bootargs");

	/* cmdline init is the part of 'prep' and nothing to do for 'bdt' */
	if (flag & BOOTM_STATE_OS_BD_T || flag & BOOTM_STATE_OS_CMDLINE)
		return -1;

	if (flag & BOOTM_STATE_OS_PREP) {
		boot_prep_linux(images);
		return 0;
	}

	if (flag & (BOOTM_STATE_OS_GO | BOOTM_STATE_OS_FAKE_GO)) {
		boot_jump_linux(images, flag);
		return 0;
	}

	boot_prep_linux(images);
	boot_jump_linux(images, flag);
	return 1;
}
