/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * CPU specific code
 */

#include <common.h>
#include <command.h>
#include <asm/system.h>

static void cache_flush(void);

int cleanup_before_linux (void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * we turn off caches etc ...
	 */

	disable_interrupts ();


	/* turn off I/D-cache */
	icache_disable();
	dcache_disable();
	l2_cache_disable();

	/* flush I/D-cache */
	cache_flush();

	return 0;
}

/* flush I/D-cache */
static void cache_flush (void)
{
#if !(defined(CONFIG_SYS_ICACHE_OFF) && defined(CONFIG_SYS_DCACHE_OFF))
	unsigned long i = 0;

	asm ("mcr p15, 0, %0, c7, c7, 0": :"r" (i));
#endif
}
