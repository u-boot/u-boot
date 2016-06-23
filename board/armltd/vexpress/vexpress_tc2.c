/*
 * (C) Copyright 2016 Linaro
 * Jon Medhurst <tixy@linaro.org>
 *
 * TC2 specific code for Versatile Express.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>

#define SCC_BASE	0x7fff0000

bool armv7_boot_nonsec_default(void)
{
#ifdef CONFIG_ARMV7_BOOT_SEC_DEFAULT
	return false
#else
	/*
	 * The Serial Configuration Controller (SCC) register at address 0x700
	 * contains flags for configuring the behaviour of the Boot Monitor
	 * (which CPUs execute from reset). Two of these bits are of interest:
	 *
	 * bit 12 = Use per-cpu mailboxes for power management
	 * bit 13 = Power down the non-boot cluster
	 *
	 * It is only when both of these are false that U-Boot's current
	 * implementation of 'nonsec' mode can work as expected because we
	 * rely on getting all CPUs to execute _nonsec_init, so let's check that.
	 */
	return (readl((u32 *)(SCC_BASE + 0x700)) & ((1 << 12) | (1 << 13))) == 0;
#endif
}
