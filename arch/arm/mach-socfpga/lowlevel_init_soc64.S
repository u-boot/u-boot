/*
 * Copyright (C) 2020 Intel Corporation. All rights reserved
 *
 * SPDX-License-Identifier:    GPL-2.0
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>

#if !defined(CONFIG_XPL_BUILD) && defined(CONFIG_ARMV8_PSCI)
.align 3
_el3_exception_vectors:
	.quad el3_exception_vectors;
#endif

ENTRY(lowlevel_init)
	mov	x29, lr			/* Save LR */

#ifdef CONFIG_XPL_BUILD
	/* Check for L2 reset magic word */
	ldr	x4, =L2_RESET_DONE_REG
	ldr	x5, [x4]
	ldr	x1, =L2_RESET_DONE_STATUS
	cmp	x1, x5
	/* No L2 reset, skip warm reset */
	b.ne	skipwarmreset
	/* Put all slaves CPUs into WFI mode */
	branch_if_slave x0, put_cpu_in_wfi
	/* L2 reset completed */
	str	xzr, [x4]
	/* Master CPU (CPU0) request for warm reset */
	mrs	x1, rmr_el3
	orr	x1, x1, #0x02
	msr	rmr_el3, x1
	isb
	dsb	sy
put_cpu_in_wfi:
	wfi
	b	put_cpu_in_wfi
skipwarmreset:
#endif

#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
#if defined(CONFIG_XPL_BUILD) && defined(CONFIG_SPL_ATF)
wait_for_atf:
	ldr	x4, =CPU_RELEASE_ADDR
	ldr	x5, [x4]
	cbz	x5, slave_wait_atf
	br	x5
slave_wait_atf:
	branch_if_slave x0, wait_for_atf
#else
	branch_if_slave x0, 1f
#endif
	ldr	x0, =GICD_BASE
	bl	gic_init_secure
1:
#if defined(CONFIG_GICV3)
	ldr	x0, =GICR_BASE
	bl	gic_init_secure_percpu
#elif defined(CONFIG_GICV2)
	ldr	x0, =GICD_BASE
	ldr	x1, =GICC_BASE
	bl	gic_init_secure_percpu
#endif
#endif

#ifdef CONFIG_ARMV8_MULTIENTRY
	branch_if_master x0, 2f

	/*
	 * Slave should wait for master clearing spin table.
	 * This sync prevent slaves observing incorrect
	 * value of spin table and jumping to wrong place.
	 */
#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
#ifdef CONFIG_GICV2
	ldr	x0, =GICC_BASE
#endif
	bl	gic_wait_for_interrupt
#endif

#if defined(CONFIG_XPL_BUILD) && !defined(CONFIG_SPL_ATF)
	/*
	 * Read the u-boot's PSCI exception handler's vector base
	 * address from the sysmgr.boot_scratch_cold6 & 7 and update
	 * their VBAR_EL3 respectively.
	 */
wait_vbar_el3:
	ldr	x4, =VBAR_EL3_BASE_ADDR
	ldr	x5, [x4]
	cbz	x5, wait_vbar_el3
	msr	vbar_el3, x5
#endif
	/*
	 * All slaves will enter EL2 and optionally EL1.
	 */
	adr	x4, lowlevel_in_el2
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el2

lowlevel_in_el2:
#ifdef CONFIG_ARMV8_SWITCH_TO_EL1
	adr	x4, lowlevel_in_el1
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el1

lowlevel_in_el1:
#endif

#endif /* CONFIG_ARMV8_MULTIENTRY */

2:
#if !defined(CONFIG_XPL_BUILD) && defined(CONFIG_ARMV8_PSCI)
	/*
	 * Write the u-boot PSCI exception handler's vector base address
	 * into a sysmgr.boot_scratch_cold6 & 7 so that other slave cpus
	 * are able to get the vector base address and update their VBAR_EL3
	 * respectively.
	 */
	adr	x0, _el3_exception_vectors
	ldr	x5, [x0]
	ldr	x4, =VBAR_EL3_BASE_ADDR
	str	x5, [x4]
#endif
	mov	lr, x29			/* Restore LR */
	ret
ENDPROC(lowlevel_init)
