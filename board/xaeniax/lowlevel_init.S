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

DRAM_SIZE:  .long   CFG_DRAM_SIZE

/* wait for coprocessor write complete */
	.macro CPWAIT reg
	mrc  p15,0,\reg,c2,c0,0
	mov  \reg,\reg
	sub  pc,pc,#4
	.endm


.globl lowlevel_init
lowlevel_init:

	mov	 r10, lr

	/* Set up GPIO pins first ----------------------------------------- */

	ldr	r0,=GPSR0
	ldr	r1,=CFG_GPSR0_VAL
	str	r1,[r0]

	ldr	r0,=GPSR1
	ldr	r1,=CFG_GPSR1_VAL
	str	r1,[r0]

	ldr	r0,=GPSR2
	ldr	r1,=CFG_GPSR2_VAL
	str	r1,[r0]

	ldr	r0,=GPCR0
	ldr	r1,=CFG_GPCR0_VAL
	str	r1,[r0]

	ldr	r0,=GPCR1
	ldr	r1,=CFG_GPCR1_VAL
	str	r1,[r0]

	ldr	r0,=GPCR2
	ldr	r1,=CFG_GPCR2_VAL
	str	r1,[r0]

	ldr	r0,=GPDR0
	ldr	r1,=CFG_GPDR0_VAL
	str	r1,[r0]

	ldr	r0,=GPDR1
	ldr	r1,=CFG_GPDR1_VAL
	str	r1,[r0]

	ldr	r0,=GPDR2
	ldr	r1,=CFG_GPDR2_VAL
	str	r1,[r0]

	ldr	r0,=GAFR0_L
	ldr	r1,=CFG_GAFR0_L_VAL
	str	r1,[r0]

	ldr	r0,=GAFR0_U
	ldr	r1,=CFG_GAFR0_U_VAL
	str	r1,[r0]

	ldr	r0,=GAFR1_L
	ldr	r1,=CFG_GAFR1_L_VAL
	str	r1,[r0]

	ldr	r0,=GAFR1_U
	ldr	r1,=CFG_GAFR1_U_VAL
	str	r1,[r0]

	ldr	r0,=GAFR2_L
	ldr	r1,=CFG_GAFR2_L_VAL
	str	r1,[r0]

	ldr	r0,=GAFR2_U
	ldr	r1,=CFG_GAFR2_U_VAL
	str	r1,[r0]

	ldr	r0,=PSSR		/* enable GPIO pins */
	ldr	r1,=CFG_PSSR_VAL
	str	r1,[r0]

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

	ldr	r3, =OSCR		/* reset the OS Timer Count to zero */
	mov	r2, #0
	str	r2, [r3]
	ldr	r4, =0x300		/* really 0x2E1 is about 200usec,   */
					/* so 0x300 should be plenty        */
1:
	ldr	r2, [r3]
	cmp	r4, r2
	bgt	1b

mem_init:

	ldr	r1,=MEMC_BASE		/* get memory controller base addr. */

	/* ---------------------------------------------------------------- */
	/* Step 2a: Initialize Asynchronous static memory controller        */
	/* ---------------------------------------------------------------- */

	/* MSC registers: timing, bus width, mem type                       */

	/* MSC0: nCS(0,1)                                                   */
	ldr     r2,=CFG_MSC0_VAL
	str     r2,[r1, #MSC0_OFFSET]
	ldr     r2,[r1, #MSC0_OFFSET]	/* read back to ensure data latches */

	/* MSC1: nCS(2,3)                                                   */
	ldr     r2,=CFG_MSC1_VAL
	str     r2,[r1, #MSC1_OFFSET]
	ldr     r2,[r1, #MSC1_OFFSET]

	/* MSC2: nCS(4,5)                                                   */
	ldr     r2,=CFG_MSC2_VAL
	str     r2,[r1, #MSC2_OFFSET]
	ldr     r2,[r1, #MSC2_OFFSET]

	/* ---------------------------------------------------------------- */
	/* Step 2b: Initialize Card Interface                               */
	/* ---------------------------------------------------------------- */

	/* MECR: Memory Expansion Card Register                             */
	ldr     r2,=CFG_MECR_VAL
	str     r2,[r1, #MECR_OFFSET]
	ldr	r2,[r1, #MECR_OFFSET]

	/* MCMEM0: Card Interface slot 0 timing                             */
	ldr     r2,=CFG_MCMEM0_VAL
	str     r2,[r1, #MCMEM0_OFFSET]
	ldr	r2,[r1, #MCMEM0_OFFSET]

	/* MCMEM1: Card Interface slot 1 timing                             */
	ldr     r2,=CFG_MCMEM1_VAL
	str     r2,[r1, #MCMEM1_OFFSET]
	ldr	r2,[r1, #MCMEM1_OFFSET]

	/* MCATT0: Card Interface Attribute Space Timing, slot 0            */
	ldr     r2,=CFG_MCATT0_VAL
	str     r2,[r1, #MCATT0_OFFSET]
	ldr	r2,[r1, #MCATT0_OFFSET]

	/* MCATT1: Card Interface Attribute Space Timing, slot 1            */
	ldr     r2,=CFG_MCATT1_VAL
	str     r2,[r1, #MCATT1_OFFSET]
	ldr	r2,[r1, #MCATT1_OFFSET]

	/* MCIO0: Card Interface I/O Space Timing, slot 0                   */
	ldr     r2,=CFG_MCIO0_VAL
	str     r2,[r1, #MCIO0_OFFSET]
	ldr	r2,[r1, #MCIO0_OFFSET]

	/* MCIO1: Card Interface I/O Space Timing, slot 1                   */
	ldr     r2,=CFG_MCIO1_VAL
	str     r2,[r1, #MCIO1_OFFSET]
	ldr	r2,[r1, #MCIO1_OFFSET]

	/* ---------------------------------------------------------------- */
	/* Step 2c: Write FLYCNFG  FIXME: what's that???                    */
	/* ---------------------------------------------------------------- */

	/* ---------------------------------------------------------------- */
	/* Step 2d: Initialize Timing for Sync Memory (SDCLK0)              */
	/* ---------------------------------------------------------------- */

	@ get the mdrefr settings
	ldr     r4,=CFG_MDREFR_VAL

	@ write back mdrefr
	str     r4,[r1, #MDREFR_OFFSET]
	ldr     r4,[r1, #MDREFR_OFFSET]

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

	@ set K1RUN for bank 0
	@
	orr   r4,  r4,  #MDREFR_K1RUN

	@ write back mdrefr
	@
	str     r4,  [r1, #MDREFR_OFFSET]
	ldr     r4,  [r1, #MDREFR_OFFSET]

	@ deassert SLFRSH
	@
	bic     r4,  r4,  #MDREFR_SLFRSH

	@ write back mdrefr
	@
	str     r4,  [r1, #MDREFR_OFFSET]
	ldr     r4,  [r1, #MDREFR_OFFSET]

	@ assert E1PIN
	@ if E0PIN is also used:	 #(MDREFR_E1PIN|MDREFR_E0PIN)
	orr     r4,  r4, #(MDREFR_E1PIN)

	@ write back mdrefr
	@
	str     r4,  [r1, #MDREFR_OFFSET]
	ldr     r4,  [r1, #MDREFR_OFFSET]
	nop
	nop

	/* Step 4d:							*/
	/* fetch platform value of mdcnfg				*/
	@
	ldr     r2,  =CFG_MDCNFG_VAL

	@ disable all sdram banks
	@
	bic     r2,  r2,  #(MDCNFG_DE0 | MDCNFG_DE1)
	bic     r2,  r2,  #(MDCNFG_DE2 | MDCNFG_DE3)

	@ program banks 0/1 for bus width
	@
	bic   r2,  r2,  #MDCNFG_DWID0      @0=32-bit

	@ write initial value of mdcnfg, w/o enabling sdram banks
	@
	str     r2,  [r1, #MDCNFG_OFFSET]

	/* Step 4e: Wait for the clock to the SDRAMs to stabilize,          */
	/*          100..200 µsec.                                          */

	ldr	r3, =OSCR		/* reset the OS Timer Count to zero */
	mov	r2, #0
	str	r2, [r3]
	ldr	r4, =0x300		/* really 0x2E1 is about 200usec,   */
					/* so 0x300 should be plenty        */
1:
	ldr	r2, [r3]
	cmp	r4, r2
	bgt	1b


	/* Step 4f: Trigger a number (usually 8) refresh cycles by          */
	/*          attempting non-burst read or write accesses to disabled */
	/*          SDRAM, as commonly specified in the power up sequence   */
	/*          documented in SDRAM data sheets. The address(es) used   */
	/*          for this purpose must not be cacheable.                 */

	ldr	r3,	=CFG_DRAM_BASE
	str	r2,	[r3]
	str	r2,	[r3]
	str	r2,	[r3]
	str	r2,	[r3]
	str	r2,	[r3]
	str	r2,	[r3]
	str	r2,	[r3]
	str	r2,	[r3]
	str	r2,	[r3]


	/* Step 4g: Write MDCNFG with enable bits asserted                  */
	/* get memory controller base address                               */
	ldr     r1,  =MEMC_BASE

	@fetch current mdcnfg value
	@
	ldr     r3,  [r1, #MDCNFG_OFFSET]

	@enable sdram bank 0 if installed (must do for any populated bank)
	@
	orr     r3,  r3,  #MDCNFG_DE0

	@write back mdcnfg, enabling the sdram bank(s)
	@
	str     r3,  [r1, #MDCNFG_OFFSET]

	/* Step 4h: Write MDMRS.                                            */

	ldr     r2,	=CFG_MDMRS_VAL
	str     r2,	[r1, #MDMRS_OFFSET]


	/* We are finished with Intel's memory controller initialisation    */


	/* ---------------------------------------------------------------- */
	/* Disable (mask) all interrupts at interrupt controller            */
	/* ---------------------------------------------------------------- */

initirqs:
	mov     r1, #0		/* clear int. level register (IRQ, not FIQ) */
	ldr     r2,  =ICLR
	str     r1,  [r2]

	ldr     r1,  =CFG_ICMR_VAL /* mask all interrupts at the controller */
	ldr     r2,  =ICMR
	str     r1,  [r2]


	/* ---------------------------------------------------------------- */
	/* Clock initialisation                                             */
	/* ---------------------------------------------------------------- */

initclks:

	/* Disable the peripheral clocks, and set the core clock frequency  */
	/* (hard-coding at 398.12MHz for now).                              */
	/* Turn Off ALL on-chip peripheral clocks for re-configuration      */
	/* Note: See label 'ENABLECLKS' for the re-enabling                 */
	ldr     r1,  =CKEN
	mov     r2,  #0
	str     r2,  [r1]


	/* default value						    */
	ldr     r2, =(CCCR_L27|CCCR_M2|CCCR_N10)  /* DEFAULT: {200/200/100} */

	/* ... and write the core clock config register                     */
	ldr     r1,  =CCCR
	str     r2,  [r1]

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

	@ Turn on needed clocks
	@
test:
	ldr     r1,  =CKEN
	ldr     r2,  =CFG_CKEN_VAL
	str     r2,  [r1]

	/* ---------------------------------------------------------------- */
	/*                                                                  */
	/* ---------------------------------------------------------------- */

	/* Save SDRAM size ?*/
	ldr	r1, =DRAM_SIZE
	str	r8, [r1]

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
	/* End lowlevel_init                                                     */
	/* ---------------------------------------------------------------- */

endlowlevel_init:

	mov     pc, lr
