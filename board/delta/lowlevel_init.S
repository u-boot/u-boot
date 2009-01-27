/*
 * (C) Copyright 2006 DENX Software Engineering
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <version.h>
#include <asm/arch/pxa-regs.h>

DRAM_SIZE:  .long   CONFIG_SYS_DRAM_SIZE

.macro wait time
	ldr		r2, =OSCR
	mov		r3, #0
	str		r3, [r2]
0:
	ldr		r3, [r2]
	cmp		r3, \time
	bls		0b
.endm

.globl lowlevel_init
lowlevel_init:
	/* Set up GPIO pins first */
	mov	 r10, lr

	/*  Configure GPIO  Pins 97, 98 UART1 / altern. Fkt. 1 */
	ldr		r0, =GPIO97
	ldr		r1, =0x801
	str		r1, [r0]

	ldr		r0, =GPIO98
	ldr		r1, =0x801
	str		r1, [r0]

	/* tebrandt - ASCR, clear the RDH bit */
	ldr		r0, =ASCR
	ldr		r1, [r0]
	bic		r1, r1, #0x80000000
	str		r1, [r0]

mem_init:
	/* Configure ACCR Register - enable DMEMC Clock at 260 / 2 MHz */
	ldr		r0, =ACCR
	ldr		r1, [r0]
	orr		r1, r1, #0x3000
	str		r1, [r0]
	ldr		r1, [r0]

	/* 2. Programm MDCNFG, leaving DMCEN de-asserted */
	ldr		r0, =MDCNFG
	ldr		r1, =(MDCNFG_DMAP | MDCNFG_DTYPE | MDCNFG_DTC_2 | MDCNFG_DCSE0 | MDCNFG_DRAC_13)
	/* ldr		r1, =0x80000403 */
	str		r1, [r0]
	ldr		r1, [r0]	/* delay until written */

	/* 3. wait nop power up waiting period (200ms)
	 * optimization: Steps 4+6 can be done during this
	 */
	wait #0x300

	/* 4. Perform an initial Rcomp-calibration cycle */
	ldr		r0, =RCOMP
	ldr		r1, =0x80000000
	str		r1, [r0]
	ldr		r1, [r0]	/* delay until written */
	/* missing: program for automatic rcomp evaluation cycles */

	/* 5. DDR DRAM strobe delay calibration */
	ldr		r0, =DDR_HCAL
	ldr		r1, =0x88000007
	str		r1, [r0]
	wait		#5
	ldr		r1, [r0]	/* delay until written */

	/* Set MDMRS */
	ldr		r0, =MDMRS
	ldr		r1, =0x60000033
	str		r1, [r0]
	wait	#300

	/* Configure MDREFR */
	ldr		r0, =MDREFR
	ldr		r1, =0x00000006
	str		r1, [r0]
	ldr		r1, [r0]

	/* Enable the dynamic memory controller */
	ldr		r0, =MDCNFG
	ldr		r1, [r0]
	orr		r1, r1, #MDCNFG_DMCEN
	str		r1, [r0]

#ifndef CONFIG_SYS_SKIP_DRAM_SCRUB
	/* scrub/init SDRAM if enabled/present */
	ldr	r8, =CONFIG_SYS_DRAM_BASE	/* base address of SDRAM (CONFIG_SYS_DRAM_BASE) */
	ldr	r9, =CONFIG_SYS_DRAM_SIZE	/* size of memory to scrub (CONFIG_SYS_DRAM_SIZE) */
	mov	r0, #0			/* scrub with 0x0000:0000 */
	mov	r1, #0
	mov	r2, #0
	mov	r3, #0
	mov	r4, #0
	mov	r5, #0
	mov	r6, #0
	mov	r7, #0
10:	/* fastScrubLoop */
	subs	r9, r9, #32	/* 8 words/line */
	stmia	r8!, {r0-r7}
	beq	15f
	b	10b
#endif /* CONFIG_SYS_SKIP_DRAM_SCRUB */

15:
	/* Mask all interrupts */
	mov	r1, #0
	mcr	p6, 0, r1, c1, c0, 0	@ ICMR

	/* Disable software and data breakpoints */
	mov	r0, #0
	mcr	p15,0,r0,c14,c8,0  /* ibcr0 */
	mcr	p15,0,r0,c14,c9,0  /* ibcr1 */
	mcr	p15,0,r0,c14,c4,0  /* dbcon */

	/* Enable all debug functionality */
	mov	r0,#0x80000000
	mcr	p14,0,r0,c10,c0,0  /* dcsr */

endlowlevel_init:
	mov	pc, lr
