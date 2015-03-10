/*
 * include/asm-arm/macro.h
 *
 * Copyright (C) 2009 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARM_MACRO_H__
#define __ASM_ARM_MACRO_H__
#ifdef __ASSEMBLY__

/*
 * These macros provide a convenient way to write 8, 16 and 32 bit data
 * to any address.
 * Registers r4 and r5 are used, any data in these registers are
 * overwritten by the macros.
 * The macros are valid for any ARM architecture, they do not implement
 * any memory barriers so caution is recommended when using these when the
 * caches are enabled or on a multi-core system.
 */

.macro	write32, addr, data
	ldr	r4, =\addr
	ldr	r5, =\data
	str	r5, [r4]
.endm

.macro	write16, addr, data
	ldr	r4, =\addr
	ldrh	r5, =\data
	strh	r5, [r4]
.endm

.macro	write8, addr, data
	ldr	r4, =\addr
	ldrb	r5, =\data
	strb	r5, [r4]
.endm

/*
 * This macro generates a loop that can be used for delays in the code.
 * Register r4 is used, any data in this register is overwritten by the
 * macro.
 * The macro is valid for any ARM architeture. The actual time spent in the
 * loop will vary from CPU to CPU though.
 */

.macro	wait_timer, time
	ldr	r4, =\time
1:
	nop
	subs	r4, r4, #1
	bcs	1b
.endm

#ifdef CONFIG_ARM64
/*
 * Register aliases.
 */
lr	.req	x30

/*
 * Branch according to exception level
 */
.macro	switch_el, xreg, el3_label, el2_label, el1_label
	mrs	\xreg, CurrentEL
	cmp	\xreg, 0xc
	b.eq	\el3_label
	cmp	\xreg, 0x8
	b.eq	\el2_label
	cmp	\xreg, 0x4
	b.eq	\el1_label
.endm

/*
 * Branch if current processor is a Cortex-A57 core.
 */
.macro	branch_if_a57_core, xreg, a57_label
	mrs	\xreg, midr_el1
	lsr	\xreg, \xreg, #4
	and	\xreg, \xreg, #0x00000FFF
	cmp	\xreg, #0xD07		/* Cortex-A57 MPCore processor. */
	b.eq	\a57_label
.endm

/*
 * Branch if current processor is a Cortex-A53 core.
 */
.macro	branch_if_a53_core, xreg, a53_label
	mrs	\xreg, midr_el1
	lsr	\xreg, \xreg, #4
	and	\xreg, \xreg, #0x00000FFF
	cmp	\xreg, #0xD03		/* Cortex-A53 MPCore processor. */
	b.eq	\a53_label
.endm

/*
 * Branch if current processor is a slave,
 * choose processor with all zero affinity value as the master.
 */
.macro	branch_if_slave, xreg, slave_label
#ifdef CONFIG_ARMV8_MULTIENTRY
	/* NOTE: MPIDR handling will be erroneous on multi-cluster machines */
	mrs	\xreg, mpidr_el1
	tst	\xreg, #0xff		/* Test Affinity 0 */
	b.ne	\slave_label
	lsr	\xreg, \xreg, #8
	tst	\xreg, #0xff		/* Test Affinity 1 */
	b.ne	\slave_label
	lsr	\xreg, \xreg, #8
	tst	\xreg, #0xff		/* Test Affinity 2 */
	b.ne	\slave_label
	lsr	\xreg, \xreg, #16
	tst	\xreg, #0xff		/* Test Affinity 3 */
	b.ne	\slave_label
#endif
.endm

/*
 * Branch if current processor is a master,
 * choose processor with all zero affinity value as the master.
 */
.macro	branch_if_master, xreg1, xreg2, master_label
#ifdef CONFIG_ARMV8_MULTIENTRY
	/* NOTE: MPIDR handling will be erroneous on multi-cluster machines */
	mrs	\xreg1, mpidr_el1
	lsr	\xreg2, \xreg1, #32
	lsl	\xreg1, \xreg1, #40
	lsr	\xreg1, \xreg1, #40
	orr	\xreg1, \xreg1, \xreg2
	cbz	\xreg1, \master_label
#else
	b 	\master_label
#endif
.endm

.macro armv8_switch_to_el2_m, xreg1
	/* 64bit EL2 | HCE | SMD | RES1 (Bits[5:4]) | Non-secure EL0/EL1 */
	mov	\xreg1, #0x5b1
	msr	scr_el3, \xreg1
	msr	cptr_el3, xzr		/* Disable coprocessor traps to EL3 */
	mov	\xreg1, #0x33ff
	msr	cptr_el2, \xreg1	/* Disable coprocessor traps to EL2 */

	/* Initialize SCTLR_EL2
	 *
	 * setting RES1 bits (29,28,23,22,18,16,11,5,4) to 1
	 * and RES0 bits (31,30,27,26,24,21,20,17,15-13,10-6) +
	 * EE,WXN,I,SA,C,A,M to 0
	 */
	mov	\xreg1, #0x0830
	movk	\xreg1, #0x30C5, lsl #16
	msr	sctlr_el2, \xreg1

	/* Return to the EL2_SP2 mode from EL3 */
	mov	\xreg1, sp
	msr	sp_el2, \xreg1		/* Migrate SP */
	mrs	\xreg1, vbar_el3
	msr	vbar_el2, \xreg1	/* Migrate VBAR */
	mov	\xreg1, #0x3c9
	msr	spsr_el3, \xreg1	/* EL2_SP2 | D | A | I | F */
	msr	elr_el3, lr
	eret
.endm

.macro armv8_switch_to_el1_m, xreg1, xreg2
	/* Initialize Generic Timers */
	mrs	\xreg1, cnthctl_el2
	orr	\xreg1, \xreg1, #0x3	/* Enable EL1 access to timers */
	msr	cnthctl_el2, \xreg1
	msr	cntvoff_el2, xzr

	/* Initilize MPID/MPIDR registers */
	mrs	\xreg1, midr_el1
	mrs	\xreg2, mpidr_el1
	msr	vpidr_el2, \xreg1
	msr	vmpidr_el2, \xreg2

	/* Disable coprocessor traps */
	mov	\xreg1, #0x33ff
	msr	cptr_el2, \xreg1	/* Disable coprocessor traps to EL2 */
	msr	hstr_el2, xzr		/* Disable coprocessor traps to EL2 */
	mov	\xreg1, #3 << 20
	msr	cpacr_el1, \xreg1	/* Enable FP/SIMD at EL1 */

	/* Initialize HCR_EL2 */
	mov	\xreg1, #(1 << 31)		/* 64bit EL1 */
	orr	\xreg1, \xreg1, #(1 << 29)	/* Disable HVC */
	msr	hcr_el2, \xreg1

	/* SCTLR_EL1 initialization
	 *
	 * setting RES1 bits (29,28,23,22,20,11) to 1
	 * and RES0 bits (31,30,27,21,17,13,10,6) +
	 * UCI,EE,EOE,WXN,nTWE,nTWI,UCT,DZE,I,UMA,SED,ITD,
	 * CP15BEN,SA0,SA,C,A,M to 0
	 */
	mov	\xreg1, #0x0800
	movk	\xreg1, #0x30d0, lsl #16
	msr	sctlr_el1, \xreg1

	/* Return to the EL1_SP1 mode from EL2 */
	mov	\xreg1, sp
	msr	sp_el1, \xreg1		/* Migrate SP */
	mrs	\xreg1, vbar_el2
	msr	vbar_el1, \xreg1	/* Migrate VBAR */
	mov	\xreg1, #0x3c5
	msr	spsr_el2, \xreg1	/* EL1_SP1 | D | A | I | F */
	msr	elr_el2, lr
	eret
.endm

#if defined(CONFIG_GICV3)
.macro gic_wait_for_interrupt_m xreg1
0 :	wfi
	mrs     \xreg1, ICC_IAR1_EL1
	msr     ICC_EOIR1_EL1, \xreg1
	cbnz    \xreg1, 0b
.endm
#elif defined(CONFIG_GICV2)
.macro gic_wait_for_interrupt_m xreg1, wreg2
0 :	wfi
	ldr     \wreg2, [\xreg1, GICC_AIAR]
	str     \wreg2, [\xreg1, GICC_AEOIR]
	and	\wreg2, \wreg2, #0x3ff
	cbnz    \wreg2, 0b
.endm
#endif

#endif /* CONFIG_ARM64 */

#endif /* __ASSEMBLY__ */
#endif /* __ASM_ARM_MACRO_H__ */
