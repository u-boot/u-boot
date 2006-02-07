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


/*
 * 	Memory setup
 */

.globl lowlevel_init
lowlevel_init:
	/* Set up GPIO pins first ----------------------------------------- */
	mov      r10, lr
	
        /*  GPIO41, 42, 43, 44, 45, 46, 47, 48 */
	ldr             r0, =0x40E10438 @ GPIO41 FFRXD
	ldr             r1, =0x802
	str             r1, [r0]

	ldr             r0, =0x40E1043C @ GPIO42 FFTXD
	ldr             r1, =0x802
	str             r1, [r0]

	ldr             r0, =0x40E10440 @ GPIO43 FFCTS
	ldr             r1, =0x802
	str             r1, [r0]

	ldr             r0, =0x40E10444 @ GPIO 44 FFDCD
	ldr             r1, =0x802
	str             r1, [r0]

	ldr             r0, =0x40E10448 @ GPIO 45 FFDSR
	ldr             r1, =0x802
	str             r1, [r0]

	ldr             r0, =0x40E1044C @ GPIO 46 FFRI
	ldr             r1, =0x802
	str             r1, [r0]

	ldr             r0, =0x40E10450 @ GPIO 47 FFDTR
	ldr             r1, =0x802
	str             r1, [r0]

	ldr             r0, =0x40E10454 @ GPIO 48
	ldr             r1, =0x802
	str             r1, [r0]

        /* tebrandt - ASCR, clear the RDH bit */
	ldr             r0, =ASCR
	ldr             r1, [r0]
	bic             r1, r1, #0x80000000
	str             r1, [r0]
	
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

	/* configure the MEMCLKCFG register */
        ldr             r1, =MEMCLKCFG
        ldr             r2, =0x00010001
        str             r2, [r1]             @ WRITE
        ldr             r2, [r1]             @ DELAY UNTIL WRITTEN
        
	/* set CSADRCFG[0] to data flash SRAM mode */
	ldr             r1, =CSADRCFG0
	ldr             r2, =0x00320809
	str             r2, [r1]             @ WRITE
	ldr             r2, [r1]             @ DELAY UNTIL WRITTEN
        
	/* set CSADRCFG[1] to data flash SRAM mode */
	ldr             r1, =CSADRCFG1
	ldr             r2, =0x00320809
	str             r2, [r1]             @ WRITE
	ldr             r2, [r1]             @ DELAY UNTIL WRITTEN
        
	/* set MSC 0 register for SRAM memory */
        ldr             r1, =MSC0
        ldr             r2, =0x11191119
        str             r2, [r1]             @ WRITE
        ldr             r2, [r1]             @ DELAY UNTIL WRITTEN
         
	/* set CSADRCFG[2] to data flash SRAM mode */
	ldr             r1, =CSADRCFG2
	ldr             r2, =0x00320809
	str             r2, [r1]             @ WRITE
	ldr             r2, [r1]             @ DELAY UNTIL WRITTEN
        
	/* set CSADRCFG[3] to VLIO mode */
        ldr             r1, =CSADRCFG3
        ldr             r2, =0x0032080B
        str             r2, [r1]             @ WRITE
        ldr             r2, [r1]             @ DELAY UNTIL WRITTEN

	/* set MSC 1 register for VLIO memory */
        ldr             r1, =MSC1
        ldr             r2, =0x123C1119
        str             r2, [r1]             @ WRITE
        ldr             r2, [r1]             @ DELAY UNTIL WRITTEN

#if 0
	/* This does not work in Zylonite. -SC */
	ldr		r0, =0x15fffff0
	ldr		r1, =0xb10b
	str		r1, [r0]
	str		r1, [r0, #4]
#endif

	/* Configure ACCR Register */
	ldr		r0, =ACCR		@ ACCR
	ldr		r1, =0x0180b108
	str		r1, [r0]
	ldr		r1, [r0]

	/* Configure MDCNFG Register */
	ldr		r0, =MDCNFG		@ MDCNFG
	ldr		r1, =0x403
	str		r1, [r0]
	ldr		r1, [r0]

	/* Perform Resistive Compensation by configuring RCOMP register */
	ldr		r1, =RCOMP		@ RCOMP
	ldr		r2, =0x000000ff
	str		r2, [r1]
	ldr		r2, [r1]

	/* Configure MDMRS Register for SDCS0 */
	ldr		r1, =MDMRS		@ MDMRS
	ldr		r2, =0x60000023
	ldr		r3, [r1]
	orr		r2, r2, r3
	str		r2, [r1]
	ldr		r2, [r1]

	/* Configure MDMRS Register for SDCS1 */
	ldr		r1, =MDMRS		@ MDMRS
	ldr		r2, =0xa0000023
	ldr		r3, [r1]
	orr		r2, r2, r3
	str		r2, [r1]
	ldr		r2, [r1]

	/* Configure MDREFR */
	ldr		r1, =MDREFR		@ MDREFR
	ldr		r2, =0x00000006
	str		r2, [r1]
	ldr		r2, [r1]

	/* Configure EMPI */
	ldr		r1, =EMPI		@ EMPI
	ldr		r2, =0x80000000
	str		r2, [r1]
	ldr		r2, [r1]

	/* DDR Read-Strobe Delay Calibration */
	/* bl	ddr_calibration */

	/* Here we assume the hardware calibration alwasy be successful. -SC */
	/* Set DMCEN bit in MDCNFG Register */
	ldr		r0, =MDCNFG		@ MDCNFG
	ldr		r1, [r0]
	orr		r1, r1, #0x40000000	@ enable SDRAM for Normal Access
	str		r1, [r0]

	/* scrub/init SDRAM if enabled/present */
	ldr	r11, =0xa0000000 //RAM_BASE	// base address of SDRAM
	ldr	r12, =0x04000000 // size of memory to scrub
	mov	r8,r12		// save DRAM size
	mov	r0, #0		// scrub with 0x0000:0000
	mov	r1, #0
	mov	r2, #0				
	mov	r3, #0
	mov	r4, #0					
	mov	r5, #0
	mov	r6, #0					
	mov	r7, #0
10:     /* fastScrubLoop */
	subs	r12, r12, #32	// 32 bytes/line
	stmia	r11!, {r0-r7}
	beq	15f
	b	10b

15:
	/* Mask all interrupts */
	mov	r1, #0
	mcr	p6, 0, r1, c1, c0, 0	@ ICMR

	/* Disable software and data breakpoints */
	mov	r0, #0
	mcr	p15,0,r0,c14,c8,0  // ibcr0
	mcr	p15,0,r0,c14,c9,0  // ibcr1
	mcr	p15,0,r0,c14,c4,0  // dbcon

	/* Enable all debug functionality */
	mov	r0,#0x80000000
	mcr	p14,0,r0,c10,c0,0  // dcsr


	
	/* We are finished with Intel's memory controller initialisation    */


	/* ---------------------------------------------------------------- */
	/* End lowlevel_init                                                     */
	/* ---------------------------------------------------------------- */

endlowlevel_init:

    mov     pc, lr
