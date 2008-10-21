/*
 * Most of this taken from Redboot hal_platform_setup.h with cleanup
 *
 * NOTE: I haven't clean this up considerably, just enough to get it
 * running. See hal_platform_setup.h for the source. See
 * board/cradle/lowlevel_init.S for another PXA250 setup that is
 * much cleaner.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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

/* wait for coprocessor write complete */
	.macro CPWAIT reg
	mrc  p15,0,\reg,c2,c0,0
	mov  \reg,\reg
	sub  pc,pc,#4
	.endm

/*
 *	Memory setup
 */
.globl lowlevel_init
lowlevel_init:

	mov      r10, lr

#ifdef DEBUG_BLINK_ENABLE
	/* 3rd blink */
	bl	blink
#endif

	/* Set up GPIO pins first ----------------------------------------- */
	ldr		r0,	=GPSR0
	ldr		r1,	=CONFIG_SYS_GPSR0_VAL
	str		r1,   [r0]

	ldr		r0,	=GPSR1
	ldr		r1,	=CONFIG_SYS_GPSR1_VAL
	str		r1,   [r0]

	ldr		r0,	=GPSR2
	ldr		r1,	=CONFIG_SYS_GPSR2_VAL
	str		r1,   [r0]

	ldr		r0,	=GPCR0
	ldr		r1,	=CONFIG_SYS_GPCR0_VAL
	str		r1,   [r0]

	ldr		r0,	=GPCR1
	ldr		r1,	=CONFIG_SYS_GPCR1_VAL
	str		r1,   [r0]

	ldr		r0,	=GPCR2
	ldr		r1,	=CONFIG_SYS_GPCR2_VAL
	str		r1,   [r0]

	ldr		r0,	=GPDR0
	ldr		r1,	=CONFIG_SYS_GPDR0_VAL
	str		r1,   [r0]

	ldr		r0,	=GPDR1
	ldr		r1,	=CONFIG_SYS_GPDR1_VAL
	str		r1,   [r0]

	ldr		r0,	=GPDR2
	ldr		r1,	=CONFIG_SYS_GPDR2_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR0_L
	ldr		r1,	=CONFIG_SYS_GAFR0_L_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR0_U
	ldr		r1,	=CONFIG_SYS_GAFR0_U_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR1_L
	ldr		r1,	=CONFIG_SYS_GAFR1_L_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR1_U
	ldr		r1,	=CONFIG_SYS_GAFR1_U_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR2_L
	ldr		r1,	=CONFIG_SYS_GAFR2_L_VAL
	str		r1,   [r0]

	ldr		r0,	=GAFR2_U
	ldr		r1,	=CONFIG_SYS_GAFR2_U_VAL
	str		r1,   [r0]

	ldr	r0,	=PSSR		/* enable GPIO pins */
	ldr		r1,	=CONFIG_SYS_PSSR_VAL
	str		r1,   [r0]

#ifdef DEBUG_BLINK_ENABLE
	/* 4th debug blink */
	bl	blink
#endif

	/* ---------------------------------------------------------------- */
	/* Enable memory interface                                          */
	/*                                                                  */
	/* The sequence below is based on the recommended init steps        */
	/* detailed in the Intel PXA250 Operating Systems Developers Guide, */
	/* Chapter 10.                                                      */
	/* ---------------------------------------------------------------- */

	/* ---------------------------------------------------------------- */
	/* Step 1: Wait for at least 200 microsedonds to allow internal     */
	/*         clocks to settle. Only necessary after hard reset...     */
	/*         FIXME: can be optimized later                            */
	/* ---------------------------------------------------------------- */

	ldr r3, =OSCR			/* reset the OS Timer Count to zero */
	mov r2, #0
	str r2, [r3]
	ldr r4, =0x300			/* really 0x2E1 is about 200usec,   */
					/* so 0x300 should be plenty        */
1:
	ldr r2, [r3]
	cmp r4, r2
	bgt 1b

mem_init:

	ldr     r1,  =MEMC_BASE		/* get memory controller base addr. */

	/* ---------------------------------------------------------------- */
	/* Step 2a: Initialize Asynchronous static memory controller        */
	/* ---------------------------------------------------------------- */

	/* MSC registers: timing, bus width, mem type                       */

	/* MSC0: nCS(0,1)                                                   */
	ldr     r2,   =CONFIG_SYS_MSC0_VAL
	str     r2,   [r1, #MSC0_OFFSET]
	ldr     r2,   [r1, #MSC0_OFFSET]	/* read back to ensure      */
						/* that data latches        */
	/* MSC1: nCS(2,3)                                                   */
	ldr     r2,  =CONFIG_SYS_MSC1_VAL
	str     r2,  [r1, #MSC1_OFFSET]
	ldr     r2,  [r1, #MSC1_OFFSET]

	/* MSC2: nCS(4,5)                                                   */
	ldr     r2,  =CONFIG_SYS_MSC2_VAL
	str     r2,  [r1, #MSC2_OFFSET]
	ldr     r2,  [r1, #MSC2_OFFSET]

	/* ---------------------------------------------------------------- */
	/* Step 2b: Initialize Card Interface                               */
	/* ---------------------------------------------------------------- */

	/* MECR: Memory Expansion Card Register                             */
	ldr     r2,  =CONFIG_SYS_MECR_VAL
	str     r2,  [r1, #MECR_OFFSET]
	ldr	r2,	[r1, #MECR_OFFSET]

	/* MCMEM0: Card Interface slot 0 timing                             */
	ldr     r2,  =CONFIG_SYS_MCMEM0_VAL
	str     r2,  [r1, #MCMEM0_OFFSET]
	ldr	r2,	[r1, #MCMEM0_OFFSET]

	/* MCMEM1: Card Interface slot 1 timing                             */
	ldr     r2,  =CONFIG_SYS_MCMEM1_VAL
	str     r2,  [r1, #MCMEM1_OFFSET]
	ldr	r2,	[r1, #MCMEM1_OFFSET]

	/* MCATT0: Card Interface Attribute Space Timing, slot 0            */
	ldr     r2,  =CONFIG_SYS_MCATT0_VAL
	str     r2,  [r1, #MCATT0_OFFSET]
	ldr	r2,	[r1, #MCATT0_OFFSET]

	/* MCATT1: Card Interface Attribute Space Timing, slot 1            */
	ldr     r2,  =CONFIG_SYS_MCATT1_VAL
	str     r2,  [r1, #MCATT1_OFFSET]
	ldr	r2,	[r1, #MCATT1_OFFSET]

	/* MCIO0: Card Interface I/O Space Timing, slot 0                   */
	ldr     r2,  =CONFIG_SYS_MCIO0_VAL
	str     r2,  [r1, #MCIO0_OFFSET]
	ldr	r2,	[r1, #MCIO0_OFFSET]

	/* MCIO1: Card Interface I/O Space Timing, slot 1                   */
	ldr     r2,  =CONFIG_SYS_MCIO1_VAL
	str     r2,  [r1, #MCIO1_OFFSET]
	ldr	r2,	[r1, #MCIO1_OFFSET]

#ifdef DEBUG_BLINK_ENABLE
	/* 5th blink */
	bl	blink
#endif

	/* ---------------------------------------------------------------- */
	/* Step 2c: Write FLYCNFG  FIXME: what's that???                    */
	/* ---------------------------------------------------------------- */

	/* ---------------------------------------------------------------- */
	/* Step 2d: Initialize Timing for Sync Memory (SDCLK0)              */
	/* ---------------------------------------------------------------- */

	/* Before accessing MDREFR we need a valid DRI field, so we set     */
	/* this to power on defaults + DRI field.                           */

	ldr     r3,     =CONFIG_SYS_MDREFR_VAL
	ldr     r2,     =0xFFF
	and     r3,     r3,  r2
	ldr	r4,	=0x03ca4000
	orr     r4,     r4,  r3
	str	r4,	[r1, #MDREFR_OFFSET]	/* write back MDREFR        */
	ldr     r4,  [r1, #MDREFR_OFFSET]

	/* Note: preserve the mdrefr value in r4                            */

	/* ---------------------------------------------------------------- */
	/* Step 3: Initialize Synchronous Static Memory (Flash/Peripherals) */
	/* ---------------------------------------------------------------- */

	/* Initialize SXCNFG register. Assert the enable bits               */

	/* Write SXMRS to cause an MRS command to all enabled banks of      */
	/* synchronous static memory. Note that SXLCR need not be written   */
	/* at this time.                                                    */

	/* FIXME: we use async mode for now                                 */

	/* ---------------------------------------------------------------- */
	/* Step 4: Initialize SDRAM                                         */
	/* ---------------------------------------------------------------- */

	/* set MDREFR according to user define with exception of a few bits */

	ldr     r4,     =CONFIG_SYS_MDREFR_VAL
	orr	r4,	r4,	#(MDREFR_SLFRSH)
	bic	r4,	r4,	#(MDREFR_E1PIN|MDREFR_E0PIN)
	str     r4,     [r1, #MDREFR_OFFSET]    /* write back MDREFR        */
	ldr     r4,  [r1, #MDREFR_OFFSET]

	/* Step 4b: de-assert MDREFR:SLFRSH.                                */

	bic	r4,	r4,	#(MDREFR_SLFRSH)
	str     r4,     [r1, #MDREFR_OFFSET]    /* write back MDREFR        */
	ldr     r4,  [r1, #MDREFR_OFFSET]

	/* Step 4c: assert MDREFR:E1PIN and E0PIO as desired                */

	ldr     r4,     =CONFIG_SYS_MDREFR_VAL
	str     r4,     [r1, #MDREFR_OFFSET]    /* write back MDREFR        */
	ldr     r4,     [r1, #MDREFR_OFFSET]


	/* Step 4d: write MDCNFG with MDCNFG:DEx deasserted (set to 0), to  */
	/*          configure but not enable each SDRAM partition pair.     */

	ldr	r4,	=CONFIG_SYS_MDCNFG_VAL
	bic	r4,	r4,	#(MDCNFG_DE0|MDCNFG_DE1)

	str     r4,     [r1, #MDCNFG_OFFSET]	/* write back MDCNFG        */
	ldr     r4,     [r1, #MDCNFG_OFFSET]

	/* Step 4e: Wait for the clock to the SDRAMs to stabilize,          */
	/*          100..200 µsec.                                          */

	ldr r3, =OSCR			/* reset the OS Timer Count to zero */
	mov r2, #0
	    str r2, [r3]
	ldr r4, =0x300			/* really 0x2E1 is about 200usec,   */
					/* so 0x300 should be plenty        */
1:
	    ldr r2, [r3]
	    cmp r4, r2
	    bgt 1b

	/* Step 4f: Trigger a number (usually 8) refresh cycles by          */
	/*          attempting non-burst read or write accesses to disabled */
	/*          SDRAM, as commonly specified in the power up sequence   */
	/*          documented in SDRAM data sheets. The address(es) used   */
	/*          for this purpose must not be cacheable.                 */

	ldr	r3,	=CONFIG_SYS_DRAM_BASE
	str	r2,	[r3]
	str	r2,	[r3]
	str	r2,	[r3]
	str	r2,	[r3]
	str	r2,	[r3]
	str	r2,	[r3]
	str	r2,	[r3]
	str	r2,	[r3]

	/* Step 4g: Write MDCNFG with enable bits asserted                  */
	/*          (MDCNFG:DEx set to 1).                                  */

	ldr     r3,  [r1, #MDCNFG_OFFSET]
	orr	r3,	r3,	#(MDCNFG_DE0|MDCNFG_DE1)
	str     r3,  [r1, #MDCNFG_OFFSET]

	/* Step 4h: Write MDMRS.                                            */

	ldr     r2,  =CONFIG_SYS_MDMRS_VAL
	str     r2,  [r1, #MDMRS_OFFSET]

	/* We are finished with Intel's memory controller initialisation    */
#if 0
	/* FIXME turn on serial ports */
	/* look into moving this to board_init() */
	ldr	r2, =(PXA_CS5_PHYS + 0x03C0002c)
	mov	r3, #0x13
	str	r3, [r2]
#endif

#ifdef DEBUG_BLINK_ENABLE
	/* 6th blink */
	bl	blink
#endif

	/* ---------------------------------------------------------------- */
	/* Disable (mask) all interrupts at interrupt controller            */
	/* ---------------------------------------------------------------- */

initirqs:

	mov     r1, #0		/* clear int. level register (IRQ, not FIQ) */
	ldr     r2,  =ICLR
	str     r1,  [r2]

	ldr     r2,  =ICMR	/* mask all interrupts at the controller    */
	str     r1,  [r2]

	/* ---------------------------------------------------------------- */
	/* Clock initialisation                                             */
	/* ---------------------------------------------------------------- */

initclks:

	/* Disable the peripheral clocks, and set the core clock frequency  */
	/* (hard-coding at 398.12MHz for now).                              */

	/* Turn Off ALL on-chip peripheral clocks for re-configuration      */
	/* Note: See label 'ENABLECLKS' for the re-enabling                 */
#if 0
	ldr     r1,  =CKEN
	mov     r2,  #0
	str     r2,  [r1]

	/* default value in case no valid rotary switch setting is found    */
	ldr     r2, =(CCCR_L27|CCCR_M2|CCCR_N10)  /* DEFAULT: {200/200/100} */

	/* ... and write the core clock config register                     */
	ldr     r1,  =CCCR
	str     r2,  [r1]

#endif

#ifdef RTC
	/* enable the 32Khz oscillator for RTC and PowerManager             */

	ldr     r1,  =OSCC
	mov     r2,  #OSCC_OON
	str     r2,  [r1]

	/* NOTE:  spin here until OSCC.OOK get set, meaning the PLL         */
	/* has settled.                                                     */
60:
	ldr     r2, [r1]
	ands    r2, r2, #1
	beq     60b
#endif

	/* ---------------------------------------------------------------- */
	/*                                                                  */
	/* ---------------------------------------------------------------- */

	/* Save SDRAM size */
	ldr     r1, =DRAM_SIZE
	str     r8, [r1]

	/* Interrupt init: Mask all interrupts                              */
	ldr	r0, =ICMR /* enable no sources */
	mov r1, #0
	str r1, [r0]

	/* FIXME */

#define NODEBUG
#ifdef NODEBUG
	/*Disable software and data breakpoints */
	mov	r0,#0
	mcr	p15,0,r0,c14,c8,0  /* ibcr0 */
	mcr	p15,0,r0,c14,c9,0  /* ibcr1 */
	mcr	p15,0,r0,c14,c4,0  /* dbcon */

	/*Enable all debug functionality */
	mov	r0,#0x80000000
	mcr	p14,0,r0,c10,c0,0  /* dcsr */
#endif

	/* ---------------------------------------------------------------- */
	/* End memsetup                                                     */
	/* ---------------------------------------------------------------- */

#ifdef DEBUG_BLINK_ENABLE
	/* 7th blink */
	bl	blink
#endif

endlowlevel_init:

	mov     pc, r10


#ifdef DEBUG_BLINK_ENABLE

/* debug LED code */

/* delay about 200ms */
delay:

	/* reset OSCR to 0 */
	ldr	r8, =OSCR
	mov	r9, #0
	str	r9, [r8]

	/* make sure new value has stuck */
1:
	ldr	r8, =OSCR
	ldr	r9, [r8]
	mov	r8, #0x10000
	cmp	r9, r8
	bgt	1b

	/* now, wait for delay to expire */
1:
	ldr	r8, =OSCR
	ldr	r9, [r8]
	mov	r8, #0xd4000
	cmp	r8, r9
	bgt	1b

	mov	pc, lr

/* blink code -- trashes r7, r8, r9 */

.globl blink
blink:

	mov	r7, lr

	/* set GPIO10 as outout */
	ldr	r8,  =GPDR0
	ldr	r9,  [r8]
	orr	r9,  r9, #(1<<10)
	str	r9,  [r8]

	/* turn LED off */
	mov	r9,  #(1<<10)
	ldr	r8,  =GPCR0
	str	r9, [r8]
	bl	delay

	/* turn LED on */
	mov	r9,  #(1<<10)
	ldr	r8,  =GPSR0
	str	r9, [r8]
	bl	delay

	/* turn LED off */
	mov	r9,  #(1<<10)
	ldr	r8,  =GPCR0
	str	r9, [r8]

	mov	pc, r7

#endif
