/*
 * (C) Copyright 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Extracted from armv8/start.S
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/gic.h>
#include <asm/macro.h>
#include "mp.h"

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

	branch_if_master x0, x1, 2f

	ldr	x0, =secondary_boot_func
	blr	x0
2:

#ifdef CONFIG_FSL_TZPC_BP147
	/* Set Non Secure access for all devices protected via TZPC */
	ldr	x1, =TZPCDECPROT_0_SET_BASE /* Decode Protection-0 Set Reg */
	orr	w0, w0, #1 << 3 /* DCFG_RESET is accessible from NS world */
	str	w0, [x1]

	isb
	dsb	sy
#endif

#ifdef CONFIG_FSL_TZASC_400
	/* Set TZASC so that:
	 * a. We use only Region0 whose global secure write/read is EN
	 * b. We use only Region0 whose NSAID write/read is EN
	 *
	 * NOTE: As per the CCSR map doc, TZASC 3 and TZASC 4 are just
	 * 	 placeholders.
	 */
	ldr	x1, =TZASC_GATE_KEEPER(0)
	ldr	x0, [x1]		/* Filter 0 Gate Keeper Register */
	orr	x0, x0, #1 << 0		/* Set open_request for Filter 0 */
	str	x0, [x1]

	ldr	x1, =TZASC_GATE_KEEPER(1)
	ldr	x0, [x1]		/* Filter 0 Gate Keeper Register */
	orr	x0, x0, #1 << 0		/* Set open_request for Filter 0 */
	str	x0, [x1]

	ldr	x1, =TZASC_REGION_ATTRIBUTES_0(0)
	ldr	x0, [x1]		/* Region-0 Attributes Register */
	orr	x0, x0, #1 << 31	/* Set Sec global write en, Bit[31] */
	orr	x0, x0, #1 << 30	/* Set Sec global read en, Bit[30] */
	str	x0, [x1]

	ldr	x1, =TZASC_REGION_ATTRIBUTES_0(1)
	ldr	x0, [x1]		/* Region-1 Attributes Register */
	orr	x0, x0, #1 << 31	/* Set Sec global write en, Bit[31] */
	orr	x0, x0, #1 << 30	/* Set Sec global read en, Bit[30] */
	str	x0, [x1]

	ldr	x1, =TZASC_REGION_ID_ACCESS_0(0)
	ldr	w0, [x1]		/* Region-0 Access Register */
	mov	w0, #0xFFFFFFFF		/* Set nsaid_wr_en and nsaid_rd_en */
	str	w0, [x1]

	ldr	x1, =TZASC_REGION_ID_ACCESS_0(1)
	ldr	w0, [x1]		/* Region-1 Attributes Register */
	mov	w0, #0xFFFFFFFF		/* Set nsaid_wr_en and nsaid_rd_en */
	str	w0, [x1]

	isb
	dsb	sy
#endif
	mov	lr, x29			/* Restore LR */
	ret
ENDPROC(lowlevel_init)

hnf_pstate_poll:
	/* x0 has the desired status, return 0 for success, 1 for timeout
	 * clobber x1, x2, x3, x4, x6, x7
	 */
	mov	x1, x0
	mov	x7, #0			/* flag for timeout */
	mrs	x3, cntpct_el0		/* read timer */
	add	x3, x3, #1200		/* timeout after 100 microseconds */
	mov	x0, #0x18
	movk	x0, #0x420, lsl #16	/* HNF0_PSTATE_STATUS */
	mov	w6, #8			/* HN-F node count */
1:
	ldr	x2, [x0]
	cmp	x2, x1			/* check status */
	b.eq	2f
	mrs	x4, cntpct_el0
	cmp	x4, x3
	b.ls	1b
	mov	x7, #1			/* timeout */
	b	3f
2:
	add	x0, x0, #0x10000	/* move to next node */
	subs	w6, w6, #1
	cbnz	w6, 1b
3:
	mov	x0, x7
	ret

hnf_set_pstate:
	/* x0 has the desired state, clobber x1, x2, x6 */
	mov	x1, x0
	/* power state to SFONLY */
	mov	w6, #8			/* HN-F node count */
	mov	x0, #0x10
	movk	x0, #0x420, lsl #16	/* HNF0_PSTATE_REQ */
1:	/* set pstate to sfonly */
	ldr	x2, [x0]
	and	x2, x2, #0xfffffffffffffffc	/* & HNFPSTAT_MASK */
	orr	x2, x2, x1
	str	x2, [x0]
	add	x0, x0, #0x10000	/* move to next node */
	subs	w6, w6, #1
	cbnz	w6, 1b

	ret

ENTRY(__asm_flush_l3_cache)
	/*
	 * Return status in x0
	 *    success 0
	 *    tmeout 1 for setting SFONLY, 2 for FAM, 3 for both
	 */
	mov	x29, lr
	mov	x8, #0

	dsb	sy
	mov	x0, #0x1		/* HNFPSTAT_SFONLY */
	bl	hnf_set_pstate

	mov	x0, #0x4		/* SFONLY status */
	bl	hnf_pstate_poll
	cbz	x0, 1f
	mov	x8, #1			/* timeout */
1:
	dsb	sy
	mov	x0, #0x3		/* HNFPSTAT_FAM */
	bl	hnf_set_pstate

	mov	x0, #0xc		/* FAM status */
	bl	hnf_pstate_poll
	cbz	x0, 1f
	add	x8, x8, #0x2
1:
	mov	x0, x8
	mov	lr, x29
	ret
ENDPROC(__asm_flush_l3_cache)

	/* Keep literals not used by the secondary boot code outside it */
	.ltorg

	/* Using 64 bit alignment since the spin table is accessed as data */
	.align 4
	.global secondary_boot_code
	/* Secondary Boot Code starts here */
secondary_boot_code:
	.global __spin_table
__spin_table:
	.space CONFIG_MAX_CPUS*SPIN_TABLE_ELEM_SIZE

	.align 2
ENTRY(secondary_boot_func)
	/*
	 * MPIDR_EL1 Fields:
	 * MPIDR[1:0] = AFF0_CPUID <- Core ID (0,1)
	 * MPIDR[7:2] = AFF0_RES
	 * MPIDR[15:8] = AFF1_CLUSTERID <- Cluster ID (0,1,2,3)
	 * MPIDR[23:16] = AFF2_CLUSTERID
	 * MPIDR[24] = MT
	 * MPIDR[29:25] = RES0
	 * MPIDR[30] = U
	 * MPIDR[31] = ME
	 * MPIDR[39:32] = AFF3
	 *
	 * Linear Processor ID (LPID) calculation from MPIDR_EL1:
	 * (We only use AFF0_CPUID and AFF1_CLUSTERID for now
	 * until AFF2_CLUSTERID and AFF3 have non-zero values)
	 *
	 * LPID = MPIDR[15:8] | MPIDR[1:0]
	 */
	mrs	x0, mpidr_el1
	ubfm	x1, x0, #8, #15
	ubfm	x2, x0, #0, #1
	orr	x10, x2, x1, lsl #2	/* x10 has LPID */
	ubfm    x9, x0, #0, #15         /* x9 contains MPIDR[15:0] */
	/*
	 * offset of the spin table element for this core from start of spin
	 * table (each elem is padded to 64 bytes)
	 */
	lsl	x1, x10, #6
	ldr	x0, =__spin_table
	/* physical address of this cpus spin table element */
	add	x11, x1, x0

	str	x9, [x11, #16]	/* LPID */
	mov	x4, #1
	str	x4, [x11, #8]	/* STATUS */
	dsb	sy
#if defined(CONFIG_GICV3)
	gic_wait_for_interrupt_m x0
#elif defined(CONFIG_GICV2)
        ldr     x0, =GICC_BASE
        gic_wait_for_interrupt_m x0, w1
#endif

	bl secondary_switch_to_el2
#ifdef CONFIG_ARMV8_SWITCH_TO_EL1
	bl secondary_switch_to_el1
#endif

slave_cpu:
	wfe
	ldr	x0, [x11]
	cbz	x0, slave_cpu
#ifndef CONFIG_ARMV8_SWITCH_TO_EL1
	mrs     x1, sctlr_el2
#else
	mrs     x1, sctlr_el1
#endif
	tbz     x1, #25, cpu_is_le
	rev     x0, x0                  /* BE to LE conversion */
cpu_is_le:
	br	x0			/* branch to the given address */
ENDPROC(secondary_boot_func)

ENTRY(secondary_switch_to_el2)
	switch_el x0, 1f, 0f, 0f
0:	ret
1:	armv8_switch_to_el2_m x0
ENDPROC(secondary_switch_to_el2)

ENTRY(secondary_switch_to_el1)
	switch_el x0, 0f, 1f, 0f
0:	ret
1:	armv8_switch_to_el1_m x0, x1
ENDPROC(secondary_switch_to_el1)

	/* Ensure that the literals used by the secondary boot code are
	 * assembled within it (this is required so that we can protect
	 * this area with a single memreserve region
	 */
	.ltorg

	/* 64 bit alignment for elements accessed as data */
	.align 4
	.globl __secondary_boot_code_size
	.type __secondary_boot_code_size, %object
	/* Secondary Boot Code ends here */
__secondary_boot_code_size:
	.quad .-secondary_boot_code
