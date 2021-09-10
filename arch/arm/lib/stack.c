// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Andreas Bießmann <andreas@biessmann.org>
 *
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 */
#include <common.h>
#include <init.h>
#include <lmb.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_reserve_stacks(void)
{
#ifdef CONFIG_SPL_BUILD
	gd->start_addr_sp -= 128;	/* leave 32 words for abort-stack */
	gd->irq_sp = gd->start_addr_sp;
#else
	/* setup stack pointer for exceptions */
	gd->irq_sp = gd->start_addr_sp;

# if !defined(CONFIG_ARM64)
	/* leave 3 words for abort-stack, plus 1 for alignment */
	gd->start_addr_sp -= 16;
# endif
#endif

	return 0;
}

static ulong get_sp(void)
{
	ulong ret;

	asm("mov %0, sp" : "=r"(ret) : );
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
		if (!gd->bd->bi_dram[bank].size ||
		    sp < gd->bd->bi_dram[bank].start)
			continue;
		/* Watch out for RAM at end of address space! */
		bank_end = gd->bd->bi_dram[bank].start +
			gd->bd->bi_dram[bank].size - 1;
		if (sp > bank_end)
			continue;
		if (bank_end > gd->ram_top)
			bank_end = gd->ram_top - 1;

		lmb_reserve(lmb, sp, bank_end - sp + 1);
		break;
	}
}
