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

/*
 * CPU specific code
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <asm/arch/ixp425.h>
#include <asm/system.h>

static void cache_flush(void);

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo (void)
{
	unsigned long id;
	int speed = 0;

	asm ("mrc p15, 0, %0, c0, c0, 0":"=r" (id));

	puts("CPU:   Intel IXP425 at ");
	switch ((id & 0x000003f0) >> 4) {
	case 0x1c:
		speed = 533;
		break;

	case 0x1d:
		speed = 400;
		break;

	case 0x1f:
		speed = 266;
		break;
	}

	if (speed)
		printf("%d MHz\n", speed);
	else
		puts("unknown revision\n");

	return 0;
}
#endif /* CONFIG_DISPLAY_CPUINFO */

int cleanup_before_linux (void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * just disable everything that can disturb booting linux
	 */

	disable_interrupts ();

	/* turn off I-cache */
	icache_disable();
	dcache_disable();

	/* flush I-cache */
	cache_flush();

	return 0;
}

/* flush I/D-cache */
static void cache_flush (void)
{
	unsigned long i = 0;

	asm ("mcr p15, 0, %0, c7, c5, 0": :"r" (i));
}

/* FIXME */
/*
void pci_init(void)
{
	return;
}
*/

int cpu_eth_init(bd_t *bis)
{
#ifdef CONFIG_IXP4XX_NPE
	npe_initialize(bis);
#endif
	return 0;
}
