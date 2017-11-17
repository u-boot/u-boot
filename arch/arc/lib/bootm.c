/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

static ulong get_sp(void)
{
	ulong ret;

	asm("mov %0, sp" : "=r"(ret) : );
	return ret;
}

void arch_lmb_reserve(struct lmb *lmb)
{
	ulong sp;

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
	lmb_reserve(lmb, sp, (CONFIG_SYS_SDRAM_BASE + gd->ram_size - sp));
}

int arch_fixup_fdt(void *blob)
{
	return 0;
}

static int cleanup_before_linux(void)
{
	disable_interrupts();
	flush_dcache_all();
	invalidate_icache_all();

	return 0;
}

/* Subcommand: PREP */
static void boot_prep_linux(bootm_headers_t *images)
{
	if (image_setup_linux(images))
		hang();
}

__weak void smp_set_core_boot_addr(unsigned long addr, int corenr) {}
__weak void smp_kick_all_cpus(void) {}

/* Subcommand: GO */
static void boot_jump_linux(bootm_headers_t *images, int flag)
{
	void (*kernel_entry)(int zero, int arch, uint params);
	unsigned int r0, r2;
	int fake = (flag & BOOTM_STATE_OS_FAKE_GO);

	kernel_entry = (void (*)(int, int, uint))images->ep;

	debug("## Transferring control to Linux (at address %08lx)...\n",
	      (ulong) kernel_entry);
	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	printf("\nStarting kernel ...%s\n\n", fake ?
	       "(fake run for tracing)" : "");
	bootstage_mark_name(BOOTSTAGE_ID_BOOTM_HANDOFF, "start_kernel");

	cleanup_before_linux();

	if (IMAGE_ENABLE_OF_LIBFDT && images->ft_len) {
		r0 = 2;
		r2 = (unsigned int)images->ft_addr;
	} else {
		r0 = 1;
		r2 = (unsigned int)env_get("bootargs");
	}

	if (!fake) {
		smp_set_core_boot_addr((unsigned long)kernel_entry, -1);
		smp_kick_all_cpus();
		kernel_entry(r0, 0, r2);
	}
}

int do_bootm_linux(int flag, int argc, char *argv[], bootm_headers_t *images)
{
	/* No need for those on ARC */
	if ((flag & BOOTM_STATE_OS_BD_T) || (flag & BOOTM_STATE_OS_CMDLINE))
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
	return 0;
}
