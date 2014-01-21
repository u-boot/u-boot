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
 * Branch if current processor is a slave,
 * choose processor with all zero affinity value as the master.
 */
.macro	branch_if_slave, xreg, slave_label
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
.endm

/*
 * Branch if current processor is a master,
 * choose processor with all zero affinity value as the master.
 */
.macro	branch_if_master, xreg1, xreg2, master_label
	mrs	\xreg1, mpidr_el1
	lsr	\xreg2, \xreg1, #32
	lsl	\xreg1, \xreg1, #40
	lsr	\xreg1, \xreg1, #40
	orr	\xreg1, \xreg1, \xreg2
	cbz	\xreg1, \master_label
.endm

#endif /* CONFIG_ARM64 */

#endif /* __ASSEMBLY__ */
#endif /* __ASM_ARM_MACRO_H__ */
