/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <version.h>
#include <linux/linkage.h>
#include <asm/macro.h>

ENTRY(armv8_switch_to_el2)
	switch_el x0, 1f, 0f, 0f
0:	ret
1:
	mov	x0, #0x5b1	/* Non-secure EL0/EL1 | HVC | 64bit EL2 */
	msr	scr_el3, x0
	msr	cptr_el3, xzr	/* Disable coprocessor traps to EL3 */
	mov	x0, #0x33ff
	msr	cptr_el2, x0	/* Disable coprocessor traps to EL2 */

	/* Initialize SCTLR_EL2 */
	msr	sctlr_el2, xzr

	/* Return to the EL2_SP2 mode from EL3 */
	mov	x0, sp
	msr	sp_el2, x0	/* Migrate SP */
	mrs	x0, vbar_el3
	msr	vbar_el2, x0	/* Migrate VBAR */
	mov	x0, #0x3c9
	msr	spsr_el3, x0	/* EL2_SP2 | D | A | I | F */
	msr	elr_el3, lr
	eret
ENDPROC(armv8_switch_to_el2)

ENTRY(armv8_switch_to_el1)
	switch_el x0, 0f, 1f, 0f
0:	ret
1:
	/* Initialize Generic Timers */
	mrs	x0, cnthctl_el2
	orr	x0, x0, #0x3		/* Enable EL1 access to timers */
	msr	cnthctl_el2, x0
	msr	cntvoff_el2, x0
	mrs	x0, cntkctl_el1
	orr	x0, x0, #0x3		/* Enable EL0 access to timers */
	msr	cntkctl_el1, x0

	/* Initilize MPID/MPIDR registers */
	mrs	x0, midr_el1
	mrs	x1, mpidr_el1
	msr	vpidr_el2, x0
	msr	vmpidr_el2, x1

	/* Disable coprocessor traps */
	mov	x0, #0x33ff
	msr	cptr_el2, x0		/* Disable coprocessor traps to EL2 */
	msr	hstr_el2, xzr		/* Disable coprocessor traps to EL2 */
	mov	x0, #3 << 20
	msr	cpacr_el1, x0		/* Enable FP/SIMD at EL1 */

	/* Initialize HCR_EL2 */
	mov	x0, #(1 << 31)		/* 64bit EL1 */
	orr	x0, x0, #(1 << 29)	/* Disable HVC */
	msr	hcr_el2, x0

	/* SCTLR_EL1 initialization */
	mov	x0, #0x0800
	movk	x0, #0x30d0, lsl #16
	msr	sctlr_el1, x0

	/* Return to the EL1_SP1 mode from EL2 */
	mov	x0, sp
	msr	sp_el1, x0		/* Migrate SP */
	mrs	x0, vbar_el2
	msr	vbar_el1, x0		/* Migrate VBAR */
	mov	x0, #0x3c5
	msr	spsr_el2, x0		/* EL1_SP1 | D | A | I | F */
	msr	elr_el2, lr
	eret
ENDPROC(armv8_switch_to_el1)
