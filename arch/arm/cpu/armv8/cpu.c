// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Texas Insturments
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 */

#include <command.h>
#include <cpu_func.h>
#include <irq_func.h>
#include <asm/cache.h>
#include <asm/system.h>
#include <asm/secure.h>
#include <linux/compiler.h>

/*
 * sdelay() - simple spin loop.
 *
 * Will delay execution by roughly (@loops * 2) cycles.
 * This is necessary to be used before timers are accessible.
 *
 * A value of "0" will results in 2^64 loops.
 */
void sdelay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %0, #1\n"
			  "b.ne 1b" : "=r" (loops) : "0"(loops) : "cc");
}

void __weak board_cleanup_before_linux(void){}

int cleanup_before_linux(void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * disable interrupt and turn off caches etc ...
	 */

	board_cleanup_before_linux();

	disable_interrupts();

	if (IS_ENABLED(CONFIG_CMO_BY_VA_ONLY)) {
		/*
		 * Disable D-cache.
		 */
		dcache_disable();
	} else {
		/*
		 * Turn off I-cache and invalidate it
		 */
		icache_disable();
		invalidate_icache_all();

		/*
		 * turn off D-cache
		 * dcache_disable() in turn flushes the d-cache and disables
		 * MMU
		 */
		dcache_disable();
		invalidate_dcache_all();
	}

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
	if (current_el() != 3)
		return;

	relocate_secure_section();
	secure_ram_addr(psci_setup_vectors)();
	secure_ram_addr(psci_arch_init)();
}
#endif

void allow_unaligned(void)
{
	set_sctlr(get_sctlr() & ~CR_A);
}
