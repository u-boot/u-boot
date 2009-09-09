/*------------------------------------------------------------------------------+
 *      This source code is dual-licensed.  You may use it under the terms of
 *      the GNU General Public License version 2, or under the license below.
 *
 *      This source code has been made available to you by IBM on an AS-IS
 *      basis.  Anyone receiving this source is licensed under IBM
 *      copyrights to use it in any way he or she deems fit, including
 *      copying it, modifying it, compiling it, and redistributing it either
 *      with or without modifications.  No license under IBM patents or
 *      patent applications is to be implied by the copyright license.
 *
 *      Any user of this software should understand that IBM cannot provide
 *      technical support for this software and will not be responsible for
 *      any consequences resulting from the use of this software.
 *
 *      Any person who transfers this source code or any derivative work
 *      must include the IBM copyright notice, this paragraph, and the
 *      preceding two paragraphs in the transferred software.
 *
 *      COPYRIGHT   I B M   CORPORATION 1995
 *      LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
 *-------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Function:     ext_bus_cntlr_init
 * Description:  Initializes the External Bus Controller for the external
 *		peripherals. IMPORTANT: For pass1 this code must run from
 *		cache since you can not reliably change a peripheral banks
 *		timing register (pbxap) while running code from that bank.
 *		For ex., since we are running from ROM on bank 0, we can NOT
 *		execute the code that modifies bank 0 timings from ROM, so
 *		we run it from cache.
 *	Bank 0 - Flash or Multi Purpose Socket
 *	Bank 1 - Multi Purpose Socket or Flash (set in C-Code)
 *	Bank 2 - UART 1 (set in C-Code)
 *	Bank 3 - UART 2 (set in C-Code)
 *	Bank 4 - not used
 *	Bank 5 - not used
 *	Bank 6 - not used
 *	Bank 7 - PLD Register
 *-----------------------------------------------------------------------------*/
#include <ppc4xx.h>

#define _LINUX_CONFIG_H 1	/* avoid reading Linux autoconf.h file	*/

#include <configs/MIP405.h>
#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>
#include "mip405.h"


  .globl ext_bus_cntlr_init
ext_bus_cntlr_init:
  mflr   r4                      /* save link register */
  mfdcr  r3,CPC0_PSR                /* get strapping reg */
  andi.  r0, r3, PSR_ROM_LOC     /* mask out irrelevant bits */
  bnelr                          /* jump back if PCI boot */

  bl      ..getAddr
..getAddr:
  mflr    r3                      /* get address of ..getAddr */
  mtlr    r4                      /* restore link register */
  addi    r4,0,14                 /* set ctr to 14; used to prefetch */
  mtctr   r4                      /* 14 cache lines to fit this function */
				   /* in cache (gives us 8x14=112 instrctns) */
..ebcloop:
  icbt    r0,r3                   /* prefetch cache line for addr in r3 */
  addi    r3,r3,32								/* move to next cache line */
  bdnz    ..ebcloop               /* continue for 14 cache lines */

   /*-------------------------------------------------------------------
    * Delay to ensure all accesses to ROM are complete before changing
    * bank 0 timings.
    *------------------------------------------------------------------- */
	addis	r3,0,0x0
  ori     r3,r3,0xA000
  mtctr   r3
..spinlp:
  bdnz    ..spinlp                /* spin loop */

	/*-----------------------------------------------------------------------
	 * decide boot up mode
	 *----------------------------------------------------------------------- */
	addi		r4,0,PB0CR
	mtdcr		EBC0_CFGADDR,r4
	mfdcr		r4,EBC0_CFGDATA

	andi.		r0, r4, 0x2000			/* mask out irrelevant bits */
	beq		0f				/* jump if 8 bit bus width */

	/* setup 16 bit things
   *-----------------------------------------------------------------------
   * Memory Bank 0 (16 Bit Flash) initialization
   *---------------------------------------------------------------------- */

	addi    r4,0,PB1AP
	mtdcr   EBC0_CFGADDR,r4
	addis   r4,0,(FLASH_AP_B)@h
	ori     r4,r4,(FLASH_AP_B)@l
	mtdcr   EBC0_CFGDATA,r4

	addi    r4,0,PB0CR
	mtdcr   EBC0_CFGADDR,r4
	/* BS=0x010(4MB),BU=0x3(R/W), */
	addis   r4,0,(FLASH_CR_B)@h
	ori     r4,r4,(FLASH_CR_B)@l
	mtdcr   EBC0_CFGDATA,r4
	b				1f

0:

	/* 8Bit boot mode: */
	/*-----------------------------------------------------------------------
	* Memory Bank 0 Multi Purpose Socket initialization
	*----------------------------------------------------------------------- */
	/* 0x7F8FFE80 slowest boot */
	addi    r4,0,PB1AP
	mtdcr   EBC0_CFGADDR,r4
	addis   r4,0,(MPS_AP_B)@h
	ori     r4,r4,(MPS_AP_B)@l
	mtdcr   EBC0_CFGDATA,r4

	addi    r4,0,PB0CR
	mtdcr   EBC0_CFGADDR,r4
	/* BS=0x010(4MB),BU=0x3(R/W), */
	addis   r4,0,(MPS_CR_B)@h
	ori     r4,r4,(MPS_CR_B)@l

	mtdcr   EBC0_CFGDATA,r4


1:
  /*-----------------------------------------------------------------------
   * Memory Bank 2-3-4-5-6 (not used) initialization
   *-----------------------------------------------------------------------*/
  addi    r4,0,PB1CR
  mtdcr   EBC0_CFGADDR,r4
  addis   r4,0,0x0000
  ori     r4,r4,0x0000
  mtdcr   EBC0_CFGDATA,r4

  addi    r4,0,PB2CR
  mtdcr   EBC0_CFGADDR,r4
  addis   r4,0,0x0000
  ori     r4,r4,0x0000
  mtdcr   EBC0_CFGDATA,r4

  addi    r4,0,PB3CR
  mtdcr   EBC0_CFGADDR,r4
  addis   r4,0,0x0000
  ori     r4,r4,0x0000
  mtdcr   EBC0_CFGDATA,r4

  addi    r4,0,PB4CR
  mtdcr   EBC0_CFGADDR,r4
  addis   r4,0,0x0000
  ori     r4,r4,0x0000
  mtdcr   EBC0_CFGDATA,r4

  addi    r4,0,PB5CR
  mtdcr   EBC0_CFGADDR,r4
  addis   r4,0,0x0000
  ori     r4,r4,0x0000
  mtdcr   EBC0_CFGDATA,r4

  addi    r4,0,PB6CR
  mtdcr   EBC0_CFGADDR,r4
  addis   r4,0,0x0000
  ori     r4,r4,0x0000
  mtdcr   EBC0_CFGDATA,r4

  addi    r4,0,PB7CR
  mtdcr   EBC0_CFGADDR,r4
  addis   r4,0,0x0000
  ori     r4,r4,0x0000
  mtdcr   EBC0_CFGDATA,r4
  nop				/* pass2 DCR errata #8 */
  blr

#if defined(CONFIG_BOOT_PCI)
    .section .bootpg,"ax"
    .globl _start_pci
/*******************************************
 */

_start_pci:
  /* first handle errata #68 / PCI_18 */
  iccci   r0, r0          /* invalidate I-cache */
  lis     r31, 0
  mticcr  r31             /* ICCR = 0 (all uncachable) */
  isync

  mfccr0  r28             /* set CCR0[24] = 1 */
  ori     r28, r28, 0x0080
  mtccr0  r28

  /* setup PMM0MA (0xEF400004) and PMM0PCIHA (0xEF40000C) */
  lis     r28, 0xEF40
  addi    r28, r28, 0x0004
  stw     r31, 0x0C(r28)  /* clear PMM0PCIHA */
  lis     r29, 0xFFF8     /* open 512 kByte */
  addi    r29, r29, 0x0001/* and enable this region */
  stwbrx  r29, r0, r28    /* write PMM0MA */

  lis     r28, 0xEEC0     /* address of PCIC0_CFGADDR */
  addi    r29, r28, 4     /* add 4 to r29 -> PCIC0_CFGDATA */

  lis     r31, 0x8000     /* set en bit bus 0 */
  ori     r31, r31, 0x304C/* device 6 func 0 reg 4C (XBCS register) */
  stwbrx  r31, r0, r28    /* write it */

  lwbrx   r31, r0, r29    /* load XBCS register */
  oris    r31, r31, 0x02C4/* clear BIOSCS WPE, set lower, extended and 1M extended BIOS enable */
  stwbrx  r31, r0, r29    /* write back XBCS register */

  nop
  nop
  b	_start		/* normal start */
#endif
