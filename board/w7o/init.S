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
	 * Setup External Bus Controller (EBC).
	 *******************************************************************/
	addi	r3, 0, epcr
	mtdcr	ebccfga, r3
	addis	r4, 0, 0xb040		/* Device base timeout = 1024 cycles */
	ori	r4, r4, 0x0		/* Drive CS with external master */
	mtdcr	ebccfgd, r4

	/********************************************************************
	 * Change PCIINT signal to PerWE
	 *******************************************************************/
	mfdcr	r4, cntrl1
	ori	r4, r4, 0x4000
	mtdcr	cntrl1, r4

	/********************************************************************
	 * Memory Bank 0 (Flash Bank 0) initialization
	 *******************************************************************/
	addi	r3, 0, pb0ap
	mtdcr	ebccfga, r3
	addis	r4, 0, CFG_W7O_EBC_PB0AP@h
	ori	r4, r4, CFG_W7O_EBC_PB0AP@l
	mtdcr	ebccfgd, r4

	addi	r3, 0, pb0cr
	mtdcr	ebccfga, r3
	addis	r4, 0, CFG_W7O_EBC_PB0CR@h
	ori	r4, r4, CFG_W7O_EBC_PB0CR@l
	mtdcr	ebccfgd, r4

	/********************************************************************
	 * Memory Bank 7 LEDs - NEEDED BECAUSE OF HW WATCHDOG AND LEDs.
	 *******************************************************************/
	addi	r3, 0, pb7ap
	mtdcr	ebccfga, r3
	addis	r4, 0, CFG_W7O_EBC_PB7AP@h
	ori	r4, r4, CFG_W7O_EBC_PB7AP@l
	mtdcr	ebccfgd, r4

	addi	r3, 0, pb7cr
	mtdcr	ebccfga, r3
	addis	r4, 0, CFG_W7O_EBC_PB7CR@h
	ori	r4, r4, CFG_W7O_EBC_PB7CR@l
	mtdcr	ebccfgd, r4

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
 *		Serial Presence Detect, "SPD," reads the SDRAM EEPROM
 *		via the IIC bus and then configures the SDRAM memory
 *		banks appropriately. If Auto Memory Configuration is
 *		is not used, it is assumed that a 4MB 11x8x2, non-ECC,
 *		SDRAM is soldered down.
 *
 * Notes:	Expects that the stack is already setup.
 *****************************************************************************/
	.section ".text"
	.align	2
	.globl	sdram_init
	.type	sdram_init, @function
sdram_init:
	/* save the return info on stack */
	mflr	r0			/* Get link register */
	stwu	r1, -8(r1)		/* Save back chain and move SP */
	stw	r0, +12(r1)		/* Save link register */

	/*
	 * First call spd_sdram to try to init SDRAM according to the
	 * contents of the SPD EEPROM.  If the SPD EEPROM is blank or
	 * erronious, spd_sdram returns 0 in R3.
	 */
	li	r3,0
	bl	spd_sdram
	addic.	r3, r3, 0		/* Check for error, save dram size */
	bne	..sdri_done		/* If it worked, we're done... */

	/********************************************************************
	 * If SPD detection fails, we'll default to 4MB, 11x8x2, as this
	 * is the SMALLEST SDRAM size the 405 supports.  We can do this
	 * because W7O boards have soldered on RAM, and there will always
	 * be some amount present.  If we were using DIMMs, we should hang
	 * the board instead, since it doesn't have any RAM to continue
	 * running with.
	 *******************************************************************/

	/*
	 * Disable memory controller to allow
	 * values to be changed.
	 */
	addi    r3, 0, mem_mcopt1
	mtdcr   memcfga, r3
	addis   r4, 0, 0x0
	ori     r4, r4, 0x0
	mtdcr   memcfgd, r4

	/*
	 * Set MB0CF for ext bank 0. (0-4MB) Address Mode 5 since 11x8x2
	 * All other banks are disabled.
	 */
	addi	r3, 0, mem_mb0cf
	mtdcr	memcfga, r3
	addis	r4, 0, 0x0000		/* BA=0x0, SZ=4MB */
	ori	r4, r4, 0x8001		/* Mode is 5, 11x8x2or4, BE=Enabled */
	mtdcr	memcfgd, r4

	/* Clear MB1CR,MB2CR,MB3CR to turn other banks off */
	addi	r4, 0, 0		/* Zero the data reg */

	addi	r3, r3, 4		/* Point to MB1CF reg */
	mtdcr	memcfga, r3		/* Set the address */
	mtdcr	memcfgd, r4		/* Zero the reg */

	addi	r3, r3, 4		/* Point to MB2CF reg */
	mtdcr	memcfga, r3		/* Set the address */
	mtdcr	memcfgd, r4		/* Zero the reg */

	addi	r3, r3, 4		/* Point to MB3CF reg */
	mtdcr	memcfga, r3		/* Set the address */
	mtdcr	memcfgd, r4		/* Zero the reg */

	/********************************************************************
	 * Set the SDRAM Timing reg, SDTR1 and the refresh timer reg, RTR.
	 * To set the appropriate timings, we assume sdram is
	 * 100MHz (pc100 compliant).
	 *******************************************************************/

	/*
	 * Set up SDTR1
	 */
	addi    r3, 0, mem_sdtr1
	mtdcr   memcfga, r3
	addis   r4, 0, 0x0086		/* SDTR1 value for 100Mhz */
	ori     r4, r4, 0x400D
	mtdcr   memcfgd, r4

	/*
	 * Set RTR
	 */
	addi    r3, 0, mem_rtr
	mtdcr   memcfga, r3
	addis   r4, 0, 0x05F0		/* RTR refresh val = 15.625ms@100Mhz */
	mtdcr   memcfgd, r4

	/********************************************************************
	 * Delay to ensure 200usec have elapsed since reset. Assume worst
	 * case that the core is running 200Mhz:
	 *	  200,000,000 (cycles/sec) X .000200 (sec) = 0x9C40 cycles
	 *******************************************************************/
	addis   r3, 0, 0x0000
	ori     r3, r3, 0xA000		/* Wait 200us from reset */
	mtctr   r3
..spinlp2:
	bdnz    ..spinlp2		/* spin loop */

	/********************************************************************
	 * Set memory controller options reg, MCOPT1.
	 *******************************************************************/
	addi    r3, 0, mem_mcopt1
	mtdcr   memcfga, r3
	addis   r4, 0, 0x80E0		/* DC_EN=1,SRE=0,PME=0,MEMCHK=0 */
	ori     r4, r4, 0x0000		/* REGEN=0,DRW=00,BRPF=01,ECCDD=1 */
	mtdcr   memcfgd, r4		/* EMDULR=1 */

..sdri_done:
	/* restore and return */
	lwz	r0, +12(r1)		/* Get saved link register */
	addi	r1, r1, +8		/* Remove frame from stack */
	mtlr	r0			/* Restore link register */
	blr				/* Return to calling function */
.Lfe1:	.size	sdram_init,.Lfe1-sdram_init
/* end sdram_init() */
