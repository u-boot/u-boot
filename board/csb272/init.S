/******************************************************************************
 *
 *	 This source code has been made available to you by IBM on an AS-IS
 *	 basis.	 Anyone receiving this source is licensed under IBM
 *	 copyrights to use it in any way he or she deems fit, including
 *	 copying it, modifying it, compiling it, and redistributing it either
 *	 with or without modifications.	 No license under IBM patents or
 *	 patent applications is to be implied by the copyright license.
 *
 *	 Any user of this software should understand that IBM cannot provide
 *	 technical support for this software and will not be responsible for
 *	 any consequences resulting from the use of this software.
 *
 *	 Any person who transfers this source code or any derivative work
 *	 must include the IBM copyright notice, this paragraph, and the
 *	 preceding two paragraphs in the transferred software.
 *
 *	 COPYRIGHT   I B M   CORPORATION 1995
 *	 LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
 *
 *****************************************************************************/
#include <config.h>
#include <ppc4xx.h>

#define _LINUX_CONFIG_H 1	/* avoid reading Linux autoconf.h file	*/

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>

#define LI32(reg,val) \
	addis   reg,0,val@h;\
	ori     reg,reg,val@l

#define WDCR_EBC(reg,val) \
	addi    r4,0,reg;\
	mtdcr   ebccfga,r4;\
	addis   r4,0,val@h;\
	ori     r4,r4,val@l;\
	mtdcr   ebccfgd,r4

#define WDCR_SDRAM(reg,val) \
	addi    r4,0,reg;\
	mtdcr   memcfga,r4;\
	addis   r4,0,val@h;\
	ori     r4,r4,val@l;\
	mtdcr   memcfgd,r4

/******************************************************************************
 * Function:	ext_bus_cntlr_init
 *
 * Description:	Configures EBC Controller and a few basic chip selects.
 *
 *		CS0 is setup to get the Boot Flash out of the addresss range
 *		so that we may setup a stack.  CS7 is setup so that we can
 *		access and reset the hardware watchdog.
 *
 *		IMPORTANT: For pass1 this code must run from
 *		cache since you can not reliably change a peripheral banks
 *		timing register (pbxap) while running code from that bank.
 *		For ex., since we are running from ROM on bank 0, we can NOT
 *		execute the code that modifies bank 0 timings from ROM, so
 *		we run it from cache.
 *
 * Notes:	Does NOT use the stack.
 *****************************************************************************/
	.section ".text"
	.align	2
	.globl	ext_bus_cntlr_init
	.type	ext_bus_cntlr_init, @function
ext_bus_cntlr_init:
	mflr	r0
	/********************************************************************
	 * Prefetch entire ext_bus_cntrl_init function into the icache.
	 * This is necessary because we are going to change the same CS we
	 * are executing from.  Otherwise a CPU lockup may occur.
	 *******************************************************************/
	bl	..getAddr
..getAddr:
	mflr	r3			/* get address of ..getAddr */

	/* Calculate number of cache lines for this function */
	addi	r4, 0, (((.Lfe0 - ..getAddr) / CFG_CACHELINE_SIZE) + 2)
	mtctr	r4
..ebcloop:
	icbt	r0, r3			/* prefetch cache line for addr in r3*/
	addi	r3, r3, CFG_CACHELINE_SIZE /* move to next cache line */
	bdnz	..ebcloop		/* continue for $CTR cache lines */

	/********************************************************************
	 * Delay to ensure all accesses to ROM are complete before changing
	 * bank 0 timings. 200usec should be enough.
	 * 200,000,000 (cycles/sec) X .000200 (sec) = 0x9C40 cycles.
	 *******************************************************************/
	addis	r3, 0, 0x0
	ori	r3, r3, 0xA000		/* wait 200us from reset */
	mtctr	r3
..spinlp:
	bdnz	..spinlp		/* spin loop */

	/********************************************************************
	 * SETUP CPC0_CR0
	 *******************************************************************/
	LI32(r4, 0x007000c0)
	mtdcr	cntrl0, r4

	/********************************************************************
	 * Setup CPC0_CR1: Change PCIINT signal to PerWE
	 *******************************************************************/
	mfdcr	r4, cntrl1
	ori	r4, r4, 0x4000
	mtdcr	cntrl1, r4

	/********************************************************************
	 * Setup External Bus Controller (EBC).
	 *******************************************************************/
	WDCR_EBC(epcr, 0xd84c0000)
	/********************************************************************
	 * Memory Bank 0 (Intel 28F128J3 Flash) initialization
	 *******************************************************************/
	/*WDCR_EBC(pb0ap, 0x02869200)*/
	WDCR_EBC(pb0ap, 0x07869200)
	WDCR_EBC(pb0cr, 0xfe0bc000)
	/********************************************************************
	 * Memory Bank 1 (Holtek HT6542B PS/2) initialization
	 *******************************************************************/
	WDCR_EBC(pb1ap, 0x1f869200)
	WDCR_EBC(pb1cr, 0xf0818000)
	/********************************************************************
	 * Memory Bank 2 (Epson S1D13506) initialization
	 *******************************************************************/
	WDCR_EBC(pb2ap, 0x05860300)
	WDCR_EBC(pb2cr, 0xf045a000)
	/********************************************************************
	 * Memory Bank 3 (Philips SJA1000 CAN Controllers) initialization
	 *******************************************************************/
	WDCR_EBC(pb3ap, 0x0387d200)
	WDCR_EBC(pb3cr, 0xf021c000)
	/********************************************************************
	 * Memory Bank 4-7 (Unused) initialization
	 *******************************************************************/
	WDCR_EBC(pb4ap, 0)
	WDCR_EBC(pb4cr, 0)
	WDCR_EBC(pb5ap, 0)
	WDCR_EBC(pb5cr, 0)
	WDCR_EBC(pb6ap, 0)
	WDCR_EBC(pb6cr, 0)
	WDCR_EBC(pb7ap, 0)
	WDCR_EBC(pb7cr, 0)

	/* We are all done */
	mtlr	r0			/* Restore link register */
	blr				/* Return to calling function */
.Lfe0:	.size	ext_bus_cntlr_init,.Lfe0-ext_bus_cntlr_init
/* end ext_bus_cntlr_init() */

/******************************************************************************
 * Function:	sdram_init
 *
 * Description:	Configures SDRAM memory banks.
 *
 * Notes:	Does NOT use the stack.
 *****************************************************************************/
	.section ".text"
	.align	2
	.globl	sdram_init
	.type	sdram_init, @function
sdram_init:

	/*
	 * Disable memory controller to allow
	 * values to be changed.
	 */
	WDCR_SDRAM(mem_mcopt1, 0x00000000)

	/*
	 * Configure Memory Banks
	 */
	WDCR_SDRAM(mem_mb0cf, 0x00084001)
	WDCR_SDRAM(mem_mb1cf, 0x00000000)
	WDCR_SDRAM(mem_mb2cf, 0x00000000)
	WDCR_SDRAM(mem_mb3cf, 0x00000000)

	/*
	 * Set up SDTR1 (SDRAM Timing Register)
	 */
	WDCR_SDRAM(mem_sdtr1, 0x00854009)

	/*
	 * Set RTR (Refresh Timing Register)
	 */
	WDCR_SDRAM(mem_rtr,   0x10000000)
	/* WDCR_SDRAM(mem_rtr,   0x05f00000) */

	/********************************************************************
	 * Delay to ensure 200usec have elapsed since reset. Assume worst
	 * case that the core is running 200Mhz:
	 *	  200,000,000 (cycles/sec) X .000200 (sec) = 0x9C40 cycles
	 *******************************************************************/
	addis   r3, 0, 0x0000
	ori     r3, r3, 0xA000		/* Wait >200us from reset */
	mtctr   r3
..spinlp2:
	bdnz    ..spinlp2		/* spin loop */

	/********************************************************************
	 * Set memory controller options reg, MCOPT1.
	 *******************************************************************/
	WDCR_SDRAM(mem_mcopt1,0x80800000)

..sdri_done:
	blr				/* Return to calling function */
.Lfe1:	.size	sdram_init,.Lfe1-sdram_init
/* end sdram_init() */
