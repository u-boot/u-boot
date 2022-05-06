/* SPDX-License-Identifier: GPL-2.0+ */

#include <config.h>
#include <linux/linkage.h>

ENTRY(arch_very_early_init)
#ifdef CONFIG_ARMADA_38X
	/*
	 * Only with disabled MMU its possible to switch the base
	 * register address on Armada 38x. Without this the SDRAM
	 * located at >= 0x4000.0000 is also not accessible, as its
	 * still locked to cache.
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, #1
	mcr	p15, 0, r0, c1, c0, 0
#endif

	/* Move internal registers from INTREG_BASE_ADDR_REG to SOC_REGS_PHY_BASE */
	ldr	r0, =SOC_REGS_PHY_BASE
	ldr	r1, =INTREG_BASE_ADDR_REG
	str	r0, [r1]
	add	r0, r0, #0xC000
	mcr	p15, 4, r0, c15, c0

	bx lr
ENDPROC(arch_very_early_init)
