/*
 * (C) Copyright 2008 Texas Insturments
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/system.h>
#include <asm/secure.h>
#include <linux/compiler.h>

int cleanup_before_linux(void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * disable interrupt and turn off caches etc ...
	 */
	disable_interrupts();

	/*
	 * Turn off I-cache and invalidate it
	 */
	icache_disable();
	invalidate_icache_all();

	/*
	 * turn off D-cache
	 * dcache_disable() in turn flushes the d-cache and disables MMU
	 */
	dcache_disable();
	invalidate_dcache_all();

	return 0;
}

#ifdef CONFIG_ARMV8_PSCI
static void relocate_secure_section(void)
{
#ifdef CONFIG_ARMV8_SECURE_BASE
	size_t sz = __secure_end - __secure_start;

	memcpy((void *)CONFIG_ARMV8_SECURE_BASE, __secure_start, sz);
	flush_dcache_range(CONFIG_ARMV8_SECURE_BASE,
			   CONFIG_ARMV8_SECURE_BASE + sz + 1);
	invalidate_icache_all();
#endif
}

void armv8_setup_psci(void)
{
	relocate_secure_section();
	secure_ram_addr(psci_setup_vectors)();
	secure_ram_addr(psci_arch_init)();
}
#endif
