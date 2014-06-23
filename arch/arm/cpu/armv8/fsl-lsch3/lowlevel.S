/*
 * (C) Copyright 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Extracted from armv8/start.S
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>

ENTRY(lowlevel_init)
	mov	x29, lr			/* Save LR */

	/* Set the SMMU page size in the sACR register */
	ldr	x1, =SMMU_BASE
	ldr	w0, [x1, #0x10]
	orr	w0, w0, #1 << 16  /* set sACR.pagesize to indicate 64K page */
	str	w0, [x1, #0x10]

	/* Initialize GIC Secure Bank Status */
#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	branch_if_slave x0, 1f
	ldr	x0, =GICD_BASE
	bl	gic_init_secure
1:
#ifdef CONFIG_GICV3
	ldr	x0, =GICR_BASE
	bl	gic_init_secure_percpu
#elif defined(CONFIG_GICV2)
	ldr	x0, =GICD_BASE
	ldr	x1, =GICC_BASE
	bl	gic_init_secure_percpu
#endif
#endif

	branch_if_master x0, x1, 1f

	/*
	 * Slave should wait for master clearing spin table.
	 * This sync prevent salves observing incorrect
	 * value of spin table and jumping to wrong place.
	 */
#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
#ifdef CONFIG_GICV2
	ldr	x0, =GICC_BASE
#endif
	bl	gic_wait_for_interrupt
#endif

	/*
	 * All processors will enter EL2 and optionally EL1.
	 */
	bl	armv8_switch_to_el2
#ifdef CONFIG_ARMV8_SWITCH_TO_EL1
	bl	armv8_switch_to_el1
#endif
	b	2f

1:
2:
	mov	lr, x29			/* Restore LR */
	ret
ENDPROC(lowlevel_init)
