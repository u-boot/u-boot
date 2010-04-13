/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/asm-offsets.h>

/*
 * L2CC Cache setup/invalidation/disable
 */
.macro init_l2cc
	/* explicitly disable L2 cache */
	mrc 15, 0, r0, c1, c0, 1
	bic r0, r0, #0x2
	mcr 15, 0, r0, c1, c0, 1

	/* reconfigure L2 cache aux control reg */
	mov r0, #0xC0			/* tag RAM */
	add r0, r0, #0x4		/* data RAM */
	orr r0, r0, #(1 << 24)		/* disable write allocate delay */
	orr r0, r0, #(1 << 23)		/* disable write allocate combine */
	orr r0, r0, #(1 << 22)		/* disable write allocate */

	cmp r3, #0x10    /* r3 contains the silicon rev */

	/* disable write combine for TO 2 and lower revs */
	orrls r0, r0, #(1 << 25)

	mcr 15, 1, r0, c9, c0, 2
.endm /* init_l2cc */

/* AIPS setup - Only setup MPROTx registers.
 * The PACR default values are good.*/
.macro init_aips
	/*
	 * Set all MPROTx to be non-bufferable, trusted for R/W,
	 * not forced to user-mode.
	 */
	ldr r0, =AIPS1_BASE_ADDR
	ldr r1, =0x77777777
	str r1, [r0, #0x0]
	str r1, [r0, #0x4]
	ldr r0, =AIPS2_BASE_ADDR
	str r1, [r0, #0x0]
	str r1, [r0, #0x4]
	/*
	 * Clear the on and off peripheral modules Supervisor Protect bit
	 * for SDMA to access them. Did not change the AIPS control registers
	 * (offset 0x20) access type
	 */
.endm /* init_aips */

/* M4IF setup */
.macro init_m4if
	/* VPU and IPU given higher priority (0x4)
	 * IPU accesses with ID=0x1 given highest priority (=0xA)
	 */
	ldr r0, =M4IF_BASE_ADDR

	ldr r1, =0x00000203
	str r1, [r0, #0x40]

	ldr r1, =0x0
	str r1, [r0, #0x44]

	ldr r1, =0x00120125
	str r1, [r0, #0x9C]

	ldr r1, =0x001901A3
	str r1, [r0, #0x48]

.endm /* init_m4if */

.macro setup_pll pll, freq
	ldr r2, =\pll
	ldr r1, =0x00001232
	str r1, [r2, #PLL_DP_CTL] /* Set DPLL ON (set UPEN bit): BRMO=1 */
	mov r1, #0x2
	str r1, [r2, #PLL_DP_CONFIG] /* Enable auto-restart AREN bit */

	str r3, [r2, #PLL_DP_OP]
	str r3, [r2, #PLL_DP_HFS_OP]

	str r4, [r2, #PLL_DP_MFD]
	str r4, [r2, #PLL_DP_HFS_MFD]

	str r5, [r2, #PLL_DP_MFN]
	str r5, [r2, #PLL_DP_HFS_MFN]

	ldr r1, =0x00001232
	str r1, [r2, #PLL_DP_CTL]
1:	ldr r1, [r2, #PLL_DP_CTL]
	ands r1, r1, #0x1
	beq 1b
.endm

.macro init_clock
	ldr r0, =CCM_BASE_ADDR

	/* Gate of clocks to the peripherals first */
	ldr r1, =0x3FFFFFFF
	str r1, [r0, #CLKCTL_CCGR0]
	ldr r1, =0x0
	str r1, [r0, #CLKCTL_CCGR1]
	str r1, [r0, #CLKCTL_CCGR2]
	str r1, [r0, #CLKCTL_CCGR3]

	ldr r1, =0x00030000
	str r1, [r0, #CLKCTL_CCGR4]
	ldr r1, =0x00FFF030
	str r1, [r0, #CLKCTL_CCGR5]
	ldr r1, =0x00000300
	str r1, [r0, #CLKCTL_CCGR6]

	/* Disable IPU and HSC dividers */
	mov r1, #0x60000
	str r1, [r0, #CLKCTL_CCDR]

	/* Make sure to switch the DDR away from PLL 1 */
	ldr r1, =0x19239145
	str r1, [r0, #CLKCTL_CBCDR]
	/* make sure divider effective */
1:	ldr r1, [r0, #CLKCTL_CDHIPR]
	cmp r1, #0x0
	bne 1b

	/* Switch ARM to step clock */
	mov r1, #0x4
	str r1, [r0, #CLKCTL_CCSR]
	mov r3, #DP_OP_800
	mov r4, #DP_MFD_800
	mov r5, #DP_MFN_800
	setup_pll PLL1_BASE_ADDR

	mov r3, #DP_OP_665
	mov r4, #DP_MFD_665
	mov r5, #DP_MFN_665
	setup_pll PLL3_BASE_ADDR

	/* Switch peripheral to PLL 3 */
	ldr r0, =CCM_BASE_ADDR
	ldr r1, =0x000010C0
	str r1, [r0, #CLKCTL_CBCMR]
	ldr r1, =0x13239145
	str r1, [r0, #CLKCTL_CBCDR]
	mov r3, #DP_OP_665
	mov r4, #DP_MFD_665
	mov r5, #DP_MFN_665
	setup_pll PLL2_BASE_ADDR

	/* Switch peripheral to PLL2 */
	ldr r0, =CCM_BASE_ADDR
	ldr r1, =0x19239145
	str r1, [r0, #CLKCTL_CBCDR]
	ldr r1, =0x000020C0
	str r1, [r0, #CLKCTL_CBCMR]

	mov r3, #DP_OP_216
	mov r4, #DP_MFD_216
	mov r5, #DP_MFN_216
	setup_pll PLL3_BASE_ADDR


	/* Set the platform clock dividers */
	ldr r0, =ARM_BASE_ADDR
	ldr r1, =0x00000725
	str r1, [r0, #0x14]

	ldr r0, =CCM_BASE_ADDR

	/* Run 3.0 at Full speed, for other TO's wait till we increase VDDGP */
	ldr r1, =0x0
	ldr r3, [r1, #ROM_SI_REV]
	cmp r3, #0x10
	movls r1, #0x1
	movhi r1, #0
	str r1, [r0, #CLKCTL_CACRR]

	/* Switch ARM back to PLL 1 */
	mov r1, #0
	str r1, [r0, #CLKCTL_CCSR]

	/* setup the rest */
	/* Use lp_apm (24MHz) source for perclk */
	ldr r1, =0x000020C2
	str r1, [r0, #CLKCTL_CBCMR]
	/* ddr clock from PLL 1, all perclk dividers are 1 since using 24MHz */
	ldr r1, =0x59E35100
	str r1, [r0, #CLKCTL_CBCDR]

	/* Restore the default values in the Gate registers */
	ldr r1, =0xFFFFFFFF
	str r1, [r0, #CLKCTL_CCGR0]
	str r1, [r0, #CLKCTL_CCGR1]
	str r1, [r0, #CLKCTL_CCGR2]
	str r1, [r0, #CLKCTL_CCGR3]
	str r1, [r0, #CLKCTL_CCGR4]
	str r1, [r0, #CLKCTL_CCGR5]
	str r1, [r0, #CLKCTL_CCGR6]

	/* Use PLL 2 for UART's, get 66.5MHz from it */
	ldr r1, =0xA5A2A020
	str r1, [r0, #CLKCTL_CSCMR1]
	ldr r1, =0x00C30321
	str r1, [r0, #CLKCTL_CSCDR1]

	/* make sure divider effective */
1:	ldr r1, [r0, #CLKCTL_CDHIPR]
	cmp r1, #0x0
	bne 1b

	mov r1, #0x0
	str r1, [r0, #CLKCTL_CCDR]

	/* for cko - for ARM div by 8 */
	mov r1, #0x000A0000
	add r1, r1, #0x00000F0
	str r1, [r0, #CLKCTL_CCOSR]
.endm

.macro setup_wdog
	ldr r0, =WDOG1_BASE_ADDR
	mov r1, #0x30
	strh r1, [r0]
.endm

.section ".text.init", "x"

.globl lowlevel_init
lowlevel_init:
	ldr r0, =GPIO1_BASE_ADDR
	ldr r1, [r0, #0x0]
	orr r1, r1, #(1 << 23)
	str r1, [r0, #0x0]
	ldr r1, [r0, #0x4]
	orr r1, r1, #(1 << 23)
	str r1, [r0, #0x4]

#ifdef ENABLE_IMPRECISE_ABORT
	mrs r1, spsr		/* save old spsr */
	mrs r0, cpsr		/* read out the cpsr */
	bic r0, r0, #0x100	/* clear the A bit */
	msr spsr, r0		/* update spsr */
	add lr, pc, #0x8	/* update lr */
	movs pc, lr		/* update cpsr */
	nop
	nop
	nop
	nop
	msr spsr, r1		/* restore old spsr */
#endif

	init_l2cc

	init_aips

	init_m4if

	init_clock

	/* r12 saved upper lr*/
	mov pc,lr

/* Board level setting value */
DDR_PERCHARGE_CMD:	.word 0x04008008
DDR_REFRESH_CMD:	.word 0x00008010
DDR_LMR1_W:		.word 0x00338018
DDR_LMR_CMD:		.word 0xB2220000
DDR_TIMING_W:		.word 0xB02567A9
DDR_MISC_W:		.word 0x000A0104
