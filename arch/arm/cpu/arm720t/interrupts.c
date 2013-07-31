/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#include <common.h>

#ifdef CONFIG_USE_IRQ
void do_irq (struct pt_regs *pt_regs)
{
}
#endif

#if defined(CONFIG_TEGRA)
static ulong timestamp;
static ulong lastdec;

int timer_init (void)
{
	/* No timer routines for tegra as yet */
	lastdec = 0;
	timestamp = 0;

	return 0;
}
#endif
