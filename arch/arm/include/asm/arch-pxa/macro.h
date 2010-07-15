/*
 * arch/arm/include/asm/arch-pxa/macro.h
 *
 * Copyright (C) 2010 Marek Vasut <marek.vasut@gmail.com>
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

#ifndef __ASM_ARCH_PXA_MACRO_H__
#define __ASM_ARCH_PXA_MACRO_H__
#ifdef __ASSEMBLY__

#include <asm/macro.h>
#include <asm/arch/pxa-regs.h>

/*
 * This macro performs a 32bit write to a memory location and makes sure the
 * write operation really happened by performing a read back.
 *
 * Clobbered regs: r4, r5
 */
.macro	write32rb addr, data
	ldr	r4, =\addr
	ldr	r5, =\data
	str	r5, [r4]
	ldr	r5, [r4]
.endm

/*
 * This macro waits according to OSCR incrementation
 *
 * Clobbered regs: r4, r5, r6
 */
.macro	pxa_wait_ticks ticks
	ldr	r4, =OSCR
	mov	r5, #0
	str	r5, [r4]
	ldr	r5, =\ticks
1:
	ldr	r6, [r4]
	cmp	r5, r6
	bgt	1b
.endm

/*
 * This macro sets up the GPIO pins of the PXA2xx/PXA3xx CPU
 *
 * Clobbered regs: r4, r5
 */
.macro	pxa_gpio_setup
	write32	GPSR0, CONFIG_SYS_GPSR0_VAL
	write32	GPSR1, CONFIG_SYS_GPSR1_VAL
	write32	GPSR2, CONFIG_SYS_GPSR2_VAL
#if defined(CONFIG_PXA27X) || defined(CONFIG_CPU_MONAHANS)
	write32	GPSR3, CONFIG_SYS_GPSR3_VAL
#endif

	write32	GPCR0, CONFIG_SYS_GPCR0_VAL
	write32	GPCR1, CONFIG_SYS_GPCR1_VAL
	write32	GPCR2, CONFIG_SYS_GPCR2_VAL
#if defined(CONFIG_PXA27X) || defined(CONFIG_CPU_MONAHANS)
	write32	GPCR3, CONFIG_SYS_GPCR3_VAL
#endif

	write32	GPDR0, CONFIG_SYS_GPDR0_VAL
	write32	GPDR1, CONFIG_SYS_GPDR1_VAL
	write32	GPDR2, CONFIG_SYS_GPDR2_VAL
#if defined(CONFIG_PXA27X) || defined(CONFIG_CPU_MONAHANS)
	write32	GPDR3, CONFIG_SYS_GPDR3_VAL
#endif

	write32	GAFR0_L, CONFIG_SYS_GAFR0_L_VAL
	write32	GAFR0_U, CONFIG_SYS_GAFR0_U_VAL
	write32	GAFR1_L, CONFIG_SYS_GAFR1_L_VAL
	write32	GAFR1_U, CONFIG_SYS_GAFR1_U_VAL
	write32	GAFR2_L, CONFIG_SYS_GAFR2_L_VAL
	write32	GAFR2_U, CONFIG_SYS_GAFR2_U_VAL
#if defined(CONFIG_PXA27X) || defined(CONFIG_CPU_MONAHANS)
	write32	GAFR3_L, CONFIG_SYS_GAFR3_L_VAL
	write32	GAFR3_U, CONFIG_SYS_GAFR3_U_VAL
#endif

	write32	PSSR, CONFIG_SYS_PSSR_VAL
.endm

/*
 * This macro sets up the Memory controller of the PXA2xx CPU
 *
 * Clobbered regs: r3, r4, r5
 */
.macro	pxa_mem_setup
	/* This comes handy when setting MDREFR */
	ldr	r3, =MEMC_BASE

	/*
	 * 1) Initialize Asynchronous static memory controller
	 */

	/* MSC0: nCS(0,1) */
	write32rb	(MEMC_BASE + MSC0_OFFSET), CONFIG_SYS_MSC0_VAL
	/* MSC1: nCS(2,3) */
	write32rb	(MEMC_BASE + MSC1_OFFSET), CONFIG_SYS_MSC1_VAL
	/* MSC2: nCS(4,5) */
	write32rb	(MEMC_BASE + MSC2_OFFSET), CONFIG_SYS_MSC2_VAL

	/*
	 * 2) Initialize Card Interface
	 */

	/* MECR: Memory Expansion Card Register */
	write32rb	(MEMC_BASE + MECR_OFFSET), CONFIG_SYS_MECR_VAL
	/* MCMEM0: Card Interface slot 0 timing */
	write32rb	(MEMC_BASE + MCMEM0_OFFSET), CONFIG_SYS_MCMEM0_VAL
	/* MCMEM1: Card Interface slot 1 timing */
	write32rb	(MEMC_BASE + MCMEM1_OFFSET), CONFIG_SYS_MCMEM1_VAL
	/* MCATT0: Card Interface Attribute Space Timing, slot 0 */
	write32rb	(MEMC_BASE + MCATT0_OFFSET), CONFIG_SYS_MCATT0_VAL
	/* MCATT1: Card Interface Attribute Space Timing, slot 1 */
	write32rb	(MEMC_BASE + MCATT1_OFFSET), CONFIG_SYS_MCATT1_VAL
	/* MCIO0: Card Interface I/O Space Timing, slot 0 */
	write32rb	(MEMC_BASE + MCIO0_OFFSET), CONFIG_SYS_MCIO0_VAL
	/* MCIO1: Card Interface I/O Space Timing, slot 1 */
	write32rb	(MEMC_BASE + MCIO1_OFFSET), CONFIG_SYS_MCIO1_VAL

	/*
	 * 3) Configure Fly-By DMA register
	 */

	write32rb	(MEMC_BASE + FLYCNFG_OFFSET), CONFIG_SYS_FLYCNFG_VAL

	/*
	 * 4) Initialize Timing for Sync Memory (SDCLK0)
	 */

	/*
	 * Before accessing MDREFR we need a valid DRI field, so we set
	 * this to power on defaults + DRI field.
	 */
	ldr	r5, [r3, #MDREFR_OFFSET]
	bic	r5, r5, #0x0ff
	bic	r5, r5, #0xf00	/* MDREFR user config with zeroed DRI */

	ldr	r4, =CONFIG_SYS_MDREFR_VAL
	mov	r6, r4
	lsl	r4, #20
	lsr	r4, #20		/* Get a valid DRI field */

	orr	r5, r5, r4	/* MDREFR user config with correct DRI */

	orr	r5, #MDREFR_K0RUN
	orr	r5, #MDREFR_SLFRSH
	bic	r5, #MDREFR_APD
	bic	r5, #MDREFR_E1PIN

	str	r5, [r3, #MDREFR_OFFSET]
	ldr	r4, [r3, #MDREFR_OFFSET]

	/*
	 * 5) Initialize Synchronous Static Memory (Flash/Peripherals)
	 */

	/* Initialize SXCNFG register. Assert the enable bits.
	 *
	 * Write SXMRS to cause an MRS command to all enabled banks of
	 * synchronous static memory. Note that SXLCR need not be written
	 * at this time.
	 */
	write32rb	(MEMC_BASE + SXCNFG_OFFSET), CONFIG_SYS_SXCNFG_VAL

	/*
	 * 6) Initialize SDRAM
	 */

	bic	r6, #MDREFR_SLFRSH
	str	r6, [r3, #MDREFR_OFFSET]
	ldr	r4, [r3, #MDREFR_OFFSET]

	orr	r6, #MDREFR_E1PIN
	str	r6, [r3, #MDREFR_OFFSET]
	ldr	r4, [r3, #MDREFR_OFFSET]

	/*
	 * 7) Write MDCNFG with MDCNFG:DEx deasserted (set to 0), to configure
	 *    but not enable each SDRAM partition pair.
	 */

	/* Fetch platform value of MDCNFG */
	ldr	r4, =CONFIG_SYS_MDCNFG_VAL
	/* Disable all sdram banks */
	bic	r4, r4, #(MDCNFG_DE0|MDCNFG_DE1)
	bic	r4, r4, #(MDCNFG_DE2|MDCNFG_DE3)
	/* Write initial value of MDCNFG, w/o enabling sdram banks */
	str	r4, [r3, #MDCNFG_OFFSET]
	ldr	r4, [r3, #MDCNFG_OFFSET]

	/* Wait for the clock to the SDRAMs to stabilize, 100..200 usec. */
	pxa_wait_ticks	0x300

	/*
	 * 8) Trigger a number (usually 8) refresh cycles by attempting
	 *    non-burst read or write accesses to disabled SDRAM, as commonly
	 *    specified in the power up sequence documented in SDRAM data
	 *    sheets. The address(es) used for this purpose must not be
	 *    cacheable.
	 */

	ldr	r4, =CONFIG_SYS_DRAM_BASE
.rept 9
	str	r5, [r4]
.endr

	/*
	 * 9) Write MDCNFG with enable bits asserted (MDCNFG:DEx set to 1).
	 */

	ldr	r5, =CONFIG_SYS_MDCNFG_VAL
	ldr	r4, =(MDCNFG_DE0 | MDCNFG_DE1 | MDCNFG_DE2 | MDCNFG_DE3)
	and	r5, r5, r4
	ldr     r4, [r3, #MDCNFG_OFFSET]
	orr	r4, r4, r5
	str     r4, [r3, #MDCNFG_OFFSET]
	ldr     r4, [r3, #MDCNFG_OFFSET]

	/*
	 * 10) Write MDMRS.
	 */

	ldr     r4, =CONFIG_SYS_MDMRS_VAL
	str     r4, [r3, #MDMRS_OFFSET]
	ldr     r4, [r3, #MDMRS_OFFSET]

	/*
	 * 11) Enable APD
	 */

	ldr	r4, [r3, #MDREFR_OFFSET]
	and	r6, r6, #MDREFR_APD
	orr	r4, r4, r6
	str	r4, [r3, #MDREFR_OFFSET]
	ldr	r4, [r3, #MDREFR_OFFSET]
.endm

/*
 * This macro tests if the CPU woke up from sleep and eventually resumes
 *
 * Clobbered regs: r4, r5
 */
.macro	pxa_wakeup
	ldr	r4, =RCSR
	ldr	r5, [r4]
	and	r5, r5, #(RCSR_GPR | RCSR_SMR | RCSR_WDR | RCSR_HWR)
	str	r5, [r4]
	teq	r5, #RCSR_SMR

	bne	pxa_wakeup_exit

	ldr	r4, =PSSR
	mov	r5, #PSSR_PH
	str	r5, [r4]

	ldr	r4, =PSPR
	ldr	pc, [r4]
pxa_wakeup_exit:
.endm

/*
 * This macro disables all interupts on PXA2xx/PXA3xx CPU
 *
 * Clobbered regs: r4, r5
 */
.macro	pxa_intr_setup
	write32	ICLR, 0
	write32	ICMR, 0
#if defined(CONFIG_PXA27X) || defined(CONFIG_CPU_MONAHANS)
	write32	ICLR2, 0
	write32	ICMR2, 0
#endif
.endm

/*
 * This macro configures clock on PXA2xx/PXA3xx CPU
 *
 * Clobbered regs: r4, r5
 */
.macro	pxa_clock_setup
	/* Disable the peripheral clocks, and set the core clock frequency */

	/* Turn Off ALL on-chip peripheral clocks for re-configuration */
	write32	CKEN, CONFIG_SYS_CKEN

	/* Write CCCR */
	write32	CCCR, CONFIG_SYS_CCCR

#ifdef CONFIG_RTC
	/* enable the 32Khz oscillator for RTC and PowerManager */
	write32	OSCC, #OSCC_OON
	ldr	r4, =OSCC

	/* Spin here until OSCC.OOK get set, meaning the PLL has settled. */
2:
	ldr	r5, [r4]
	ands	r5, r5, #1
	beq	2b
#endif
.endm

#endif /* __ASSEMBLY__ */
#endif /* __ASM_ARCH_PXA_MACRO_H__ */
