/*----------------------------------------------------------------------+
 *       This source code is dual-licensed.  You may use it under the terms of
 *       the GNU General Public License version 2, or under the license below.
 *
 *       This source code has been made available to you by IBM on an AS-IS
 *       basis.  Anyone receiving this source is licensed under IBM
 *       copyrights to use it in any way he or she deems fit, including
 *       copying it, modifying it, compiling it, and redistributing it either
 *       with or without modifications.  No license under IBM patents or
 *       patent applications is to be implied by the copyright license.
 *
 *       Any user of this software should understand that IBM cannot provide
 *       technical support for this software and will not be responsible for
 *       any consequences resulting from the use of this software.
 *
 *       Any person who transfers this source code or any derivative work
 *       must include the IBM copyright notice, this paragraph, and the
 *       preceding two paragraphs in the transferred software.
 *
 *       COPYRIGHT   I B M   CORPORATION 1995
 *       LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
 *-----------------------------------------------------------------------
 */

#include <config.h>
#include <ppc4xx.h>
#include "config.h"

#define _LINUX_CONFIG_H 1	/* avoid reading Linux autoconf.h file	*/
#define FPGA_BRDC       0xF0300004

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>

#include "exbitgen.h"

/* IIC declarations (This is an extract from 405gp_i2c.h, which also contains some */
/* c-code declarations and consequently can't be included here). */
/* (Possibly to be solved somehow else). */
/*--------------------------------------------------------------------- */
#define	   I2C_REGISTERS_BASE_ADDRESS 0xEF600500
#define    IIC_MDBUF	(I2C_REGISTERS_BASE_ADDRESS+IICMDBUF)
#define    IIC_SDBUF	(I2C_REGISTERS_BASE_ADDRESS+IICSDBUF)
#define    IIC_LMADR	(I2C_REGISTERS_BASE_ADDRESS+IICLMADR)
#define    IIC_HMADR	(I2C_REGISTERS_BASE_ADDRESS+IICHMADR)
#define    IIC_CNTL	(I2C_REGISTERS_BASE_ADDRESS+IICCNTL)
#define    IIC_MDCNTL	(I2C_REGISTERS_BASE_ADDRESS+IICMDCNTL)
#define    IIC_STS	(I2C_REGISTERS_BASE_ADDRESS+IICSTS)
#define    IIC_EXTSTS	(I2C_REGISTERS_BASE_ADDRESS+IICEXTSTS)
#define    IIC_LSADR	(I2C_REGISTERS_BASE_ADDRESS+IICLSADR)
#define    IIC_HSADR	(I2C_REGISTERS_BASE_ADDRESS+IICHSADR)
#define    IIC_CLKDIV	(I2C_REGISTERS_BASE_ADDRESS+IIC0_CLKDIV)
#define    IIC_INTRMSK	(I2C_REGISTERS_BASE_ADDRESS+IICINTRMSK)
#define    IIC_XFRCNT	(I2C_REGISTERS_BASE_ADDRESS+IICXFRCNT)
#define    IIC_XTCNTLSS	(I2C_REGISTERS_BASE_ADDRESS+IICXTCNTLSS)
#define    IIC_DIRECTCNTL (I2C_REGISTERS_BASE_ADDRESS+IICDIRECTCNTL)

/* MDCNTL Register Bit definition */
#define    IIC_MDCNTL_HSCL 0x01
#define    IIC_MDCNTL_EUBS 0x02
#define    IIC_MDCNTL_FMDB 0x40
#define    IIC_MDCNTL_FSDB 0x80

/* CNTL Register Bit definition */
#define    IIC_CNTL_PT     0x01
#define    IIC_CNTL_READ   0x02
#define    IIC_CNTL_CHT    0x04

/* STS Register Bit definition */
#define    IIC_STS_PT	   0X01
#define    IIC_STS_ERR	   0X04
#define    IIC_STS_MDBS    0X20

/* EXTSTS Register Bit definition */
#define    IIC_EXTSTS_XFRA 0X01
#define    IIC_EXTSTS_ICT  0X02
#define    IIC_EXTSTS_LA   0X04

/* LED codes used for inditing progress and errors during read of DIMM SPD.  */
/*--------------------------------------------------------------------- */
#define LED_SDRAM_CODE_1  0xef
#define LED_SDRAM_CODE_2  0xee
#define LED_SDRAM_CODE_3  0xed
#define LED_SDRAM_CODE_4  0xec
#define LED_SDRAM_CODE_5  0xeb
#define LED_SDRAM_CODE_6  0xea
#define LED_SDRAM_CODE_7  0xe9
#define LED_SDRAM_CODE_8  0xe8
#define LED_SDRAM_CODE_9  0xe7
#define LED_SDRAM_CODE_10 0xe6
#define LED_SDRAM_CODE_11 0xe5
#define LED_SDRAM_CODE_12 0xe4
#define LED_SDRAM_CODE_13 0xe3
#define LED_SDRAM_CODE_14 0xe2
#define LED_SDRAM_CODE_15 0xe1
#define LED_SDRAM_CODE_16 0xe0


#define TIMEBASE_10PS (1000000000 / CONFIG_SYS_CLK_FREQ) * 100

#define FLASH_8bit_AP   0x9B015480
#define FLASH_8bit_CR   0xFFF18000 /* 1MB(min), 8bit, R/W */

#define FLASH_32bit_AP  0x9B015480
#define FLASH_32bit_CR  0xFFE3C000 /* 2MB, 32bit, R/W */


#define WDCR_EBC(reg,val) addi    r4,0,reg;\
	mtdcr   EBC0_CFGADDR,r4;\
	addis   r4,0,val@h;\
	ori     r4,r4,val@l;\
	mtdcr   EBC0_CFGDATA,r4

/*---------------------------------------------------------------------
 * Function:     ext_bus_cntlr_init
 * Description:  Initializes the External Bus Controller for the external
 *		peripherals. IMPORTANT: For pass1 this code must run from
 *		cache since you can not reliably change a peripheral banks
 *		timing register (pbxap) while running code from that bank.
 *		For ex., since we are running from ROM on bank 0, we can NOT
 *		execute the code that modifies bank 0 timings from ROM, so
 *		we run it from cache.
 *	Bank 0 - Boot flash
 *	Bank 1-4 - application flash
 *	Bank 5 - CPLD
 *	Bank 6 - not used
 *	Bank 7 - Heathrow chip
 *---------------------------------------------------------------------
 */
	.globl	ext_bus_cntlr_init
ext_bus_cntlr_init:
	mflr    r4                      /* save link register */
	bl      ..getAddr
..getAddr:
	mflr    r3                      /* get address of ..getAddr */
	mtlr    r4                      /* restore link register */
	addi    r4,0,14                 /* set ctr to 10; used to prefetch */
	mtctr   r4                      /* 10 cache lines to fit this function */
					/* in cache (gives us 8x10=80 instrctns) */
..ebcloop:
	icbt    r0,r3                   /* prefetch cache line for addr in r3 */
	addi    r3,r3,32		/* move to next cache line */
	bdnz    ..ebcloop               /* continue for 10 cache lines */

	mflr	r31			/* save link register */

	/*-----------------------------------------------------------
	 * Delay to ensure all accesses to ROM are complete before changing
	 * bank 0 timings. 200usec should be enough.
	 *   200,000,000 (cycles/sec) X .000200 (sec) = 0x9C40 cycles
	 *-----------------------------------------------------------
	 */

	addis	r3,0,0x0
	ori     r3,r3,0xA000          /* ensure 200usec have passed since reset */
	mtctr   r3
..spinlp:
	bdnz    ..spinlp                /* spin loop */

	/*---------------------------------------------------------------
	 * Memory Bank 0 (Boot Flash) initialization
	 *---------------------------------------------------------------
	 */
	WDCR_EBC(PB1AP, FLASH_32bit_AP)
	WDCR_EBC(PB0CR, 0xffe38000)
/*pnc	WDCR_EBC(PB0CR, FLASH_32bit_CR) */

	/*---------------------------------------------------------------
	 * Memory Bank 5 (CPLD) initialization
	 *---------------------------------------------------------------
	 */
	WDCR_EBC(PB5AP, 0x01010040)
/*jsa recommendation:		WDCR_EBC(PB5AP, 0x00010040) */
	WDCR_EBC(PB5CR, 0x10038000)

	/*--------------------------------------------------------------- */
	/* Memory Bank 6 (not used) initialization */
	/*--------------------------------------------------------------- */
	WDCR_EBC(PB6CR, 0x00000000)

	/* Read HW ID to determine whether old H2 board or new generic CPU board */
	addis	r3, 0,  HW_ID_ADDR@h
	ori	r3, r3, HW_ID_ADDR@l
	lbz     r3,0x0000(r3)
	cmpi	0, r3, 1          /* if (HW_ID==1) */
	beq	setup_h2evalboard /* then jump */
	cmpi	0, r3, 2          /* if (HW_ID==2) */
	beq	setup_genieboard  /* then jump */
	cmpi	0, r3, 3          /* if (HW_ID==3) */
	beq	setup_genieboard  /* then jump */

setup_genieboard:
	/*--------------------------------------------------------------- */
	/* Memory Bank 1 (Application Flash) initialization for generic CPU board */
	/*--------------------------------------------------------------- */
/*	WDCR_EBC(PB1AP, 0x7b015480)	/###* T.B.M. */
/*	WDCR_EBC(PB1AP, 0x7F8FFE80)	/###* T.B.M. */
	WDCR_EBC(PB1AP, 0x9b015480)	/* hlb-20020207: burst 8 bit 6 cycles  */

/*	WDCR_EBC(PB1CR, 0x20098000)	/###* 16 MB */
	WDCR_EBC(PB1CR, 0x200B8000)	/* 32 MB */

	/*--------------------------------------------------------------- */
	/* Memory Bank 4 (Onboard FPGA) initialization for generic CPU board */
	/*--------------------------------------------------------------- */
	WDCR_EBC(PB4AP, 0x01010000)	/*  */
	WDCR_EBC(PB4CR, 0x1021c000)	/*  */

	/*--------------------------------------------------------------- */
	/* Memory Bank 7 (Heathrow chip on Reference board) initialization */
	/*--------------------------------------------------------------- */
	WDCR_EBC(PB7AP, 0x200ffe80)	/* No Ready, many wait states (let reflections die out) */
	WDCR_EBC(PB7CR, 0X4001A000)

	bl	setup_continue


setup_h2evalboard:
	/*--------------------------------------------------------------- */
	/* Memory Bank 1 (Application Flash) initialization */
	/*--------------------------------------------------------------- */
	WDCR_EBC(PB1AP, 0x7b015480)	/* T.B.M. */
/*3010	WDCR_EBC(PB1AP, 0x7F8FFE80)	/###* T.B.M. */
	WDCR_EBC(PB1CR, 0x20058000)

	/*--------------------------------------------------------------- */
	/* Memory Bank 2 (Application Flash) initialization */
	/*--------------------------------------------------------------- */
	WDCR_EBC(PB2AP, 0x7b015480)	/* T.B.M. */
/*3010	WDCR_EBC(PB2AP, 0x7F8FFE80)	/###* T.B.M. */
	WDCR_EBC(PB2CR, 0x20458000)

	/*--------------------------------------------------------------- */
	/* Memory Bank 3 (Application Flash) initialization */
	/*--------------------------------------------------------------- */
	WDCR_EBC(PB3AP, 0x7b015480)	/* T.B.M. */
/*3010	WDCR_EBC(PB3AP, 0x7F8FFE80)	/###* T.B.M. */
	WDCR_EBC(PB3CR, 0x20858000)

	/*--------------------------------------------------------------- */
	/* Memory Bank 4 (Application Flash) initialization */
	/*--------------------------------------------------------------- */
	WDCR_EBC(PB4AP, 0x7b015480)	/* T.B.M. */
/*3010	WDCR_EBC(PB4AP, 0x7F8FFE80)	/###* T.B.M. */
	WDCR_EBC(PB4CR, 0x20C58000)

	/*--------------------------------------------------------------- */
	/* Memory Bank 7 (Heathrow chip) initialization */
	/*--------------------------------------------------------------- */
	WDCR_EBC(PB7AP, 0x02000280)	/* No Ready, 4 wait states */
	WDCR_EBC(PB7CR, 0X4001A000)

setup_continue:


	mtlr    r31                     /* restore lr	 */
	nop				/* pass2 DCR errata #8 */
	blr

/*--------------------------------------------------------------------- */
/* Function:     sdram_init */
/* Description:  Configures SDRAM memory banks. */
/*--------------------------------------------------------------------- */
	.globl  sdram_init

sdram_init:
#if CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE
	blr
#else
	mflr	r31

	/* output SDRAM code  on LEDs */
	addi	r4, 0, LED_SDRAM_CODE_1
	addis	r5, 0, 0x1000
	ori	r5, r5, 0x0001
	stb	r4,0(r5)
	eieio

	/* Read contents of spd */
	/*--------------------- */
	bl	read_spd

	/*----------------------------------------------------------- */
	/* */
	/* */
	/* Update SDRAM timing register */
	/* */
	/* */
	/*----------------------------------------------------------- */

	/* Read  PLL feedback divider and calculate clock period of local bus in */
	/* granularity of 10 ps. Save clock period in r30 */
	/*-------------------------------------------------------------- */
	mfdcr	r4, CPC0_PLLMR
	addi	r9, 0, 25
	srw	r4, r4, r9
	andi.	r4, r4, 0x07
	addis	r5, 0,  TIMEBASE_10PS@h
	ori	r5, r5, TIMEBASE_10PS@l
	divwu	r30, r5, r4

	/* Determine CASL */
	/*--------------- */
	bl	find_casl	/* Returns CASL in r3 */

	/* Calc trp_clocks = (trp * 100 + (clk - 1)) / clk */
	/* (trp read from byte 27 in granularity of 1 ns) */
	/*------------------------------------------------ */
	mulli	r16, r16, 100
	add	r16, r16, r30
	addi	r6, 0, 1
	subf	r16, r6, r16
	divwu	r16, r16, r30

	/* Calc trcd_clocks = (trcd * 100 + (clk - 1) ) / clk */
	/* (trcd read from byte 29 in granularity of 1 ns) */
	/*--------------------------------------------------- */
	mulli	r17, r17, 100
	add	r17, r17, r30
	addi	r6, 0, 1
	subf	r17, r6, r17
	divwu	r17, r17, r30

	/* Calc tras_clocks = (tras * 100 + (clk - 1) ) / clk */
	/* (tras read from byte 30 in granularity of 1 ns) */
	/*--------------------------------------------------- */
	mulli	r18, r18, 100
	add	r18, r18, r30
	addi	r6, 0, 1
	subf	r18, r6, r18
	divwu	r18, r18, r30

	/* Calc trc_clocks = trp_clocks + tras_clocks */
	/*------------------------------------------- */
	add	r18, r18, r16

	/* CASL value */
	/*----------- */
	addi	r9, 0, 23
	slw	r4, r3, r9

	/* PTA = trp_clocks - 1 */
	/*--------------------- */
	addi	r6, 0, 1
	subf	r5, r6, r16
	addi	r9, 0, 18
	slw	r5, r5, r9
	or	r4, r4, r5

	/* CTP = trc_clocks - trp_clocks - trcd_clocks - 1 */
	/*------------------------------------------------ */
	addi	r5, r18, 0
	subf	r5, r16, r5
	subf	r5, r17, r5
	addi	r6, 0, 1
	subf	r5, r6, r5
	addi	r9, 0, 16
	slw	r5, r5, r9
	or	r4, r4, r5

	/* LDF = 1 */
	/*-------- */
	ori	r4, r4, 0x4000

	/* RFTA = trc_clocks - 4 */
	/*---------------------- */
	addi	r6, 0, 4
	subf	r5, r6, r18
	addi	r9, 0, 2
	slw	r5, r5, r9
	or	r4, r4, r5

	/* RCD = trcd_clocks - 1 */
	/*---------------------- */
	addi	r6, 0, 1
	subf	r5, r6, r17
	or	r4, r4, r5

	/*----------------------------------------------------------- */
	/* Set SDTR1  */
	/*----------------------------------------------------------- */
	addi    r5,0,SDRAM0_TR
	mtdcr   SDRAM0_CFGADDR,r5
	mtdcr   SDRAM0_CFGDATA,r4

	/*----------------------------------------------------------- */
	/* */
	/* */
	/* Update memory bank 0-3 configuration registers */
	/* */
	/* */
	/*----------------------------------------------------------- */

	/* Build contents of configuration register for bank 0 into r6 */
	/*------------------------------------------------------------ */
	bl	find_mode	/* returns addressing mode in r3 */
	addi	r29, r3, 0	/* save mode temporarily in r29 */
	bl	find_size_code	/* returns size code in r3 */
	addi	r9, 0, 17	/* bit offset of size code in configuration register */
	slw	r3, r3, r9	/* */
	addi	r9, 0, 13	/* bit offset of addressing mode in configuration register  */
	slw	r29, r29, r9	/*  */
	or	r3, r29, r3	/* merge size code and addressing mode */
	ori	r6, r3, CONFIG_SYS_SDRAM_BASE + 1 /* insert base address and enable bank */

	/* Calculate banksize r15 = (density << 22) / 2 */
	/*--------------------------------------------- */
	addi	r9, 0, 21
	slw	r15, r15, r9

	/* Set SDRAM bank 0 register and adjust r6 for next bank */
	/*------------------------------------------------------ */
	addi    r7,0,SDRAM0_B0CR
	mtdcr   SDRAM0_CFGADDR,r7
	mtdcr   SDRAM0_CFGDATA,r6

	add	r6, r6, r15	/* add bank size to base address for next bank */

	/* If two rows/banks then set SDRAM bank 1 register and adjust r6 for next bank */
	/*---------------------------------------------------------------------------- */
	cmpi	0, r12, 2
	bne	b1skip

	addi    r7,0,SDRAM0_B1CR
	mtdcr   SDRAM0_CFGADDR,r7
	mtdcr   SDRAM0_CFGDATA,r6

	add	r6, r6, r15	/* add bank size to base address for next bank */

	/* Set SDRAM bank 2 register and adjust r6 for next bank */
	/*------------------------------------------------------ */
b1skip:	addi    r7,0,SDRAM0_B2CR
	mtdcr   SDRAM0_CFGADDR,r7
	mtdcr   SDRAM0_CFGDATA,r6

	add	r6, r6, r15	/* add bank size to base address for next bank */

	/* If two rows/banks then set SDRAM bank 3 register */
	/*------------------------------------------------ */
	cmpi	0, r12, 2
	bne	b3skip

	addi    r7,0,SDRAM0_B3CR
	mtdcr   SDRAM0_CFGADDR,r7
	mtdcr   SDRAM0_CFGDATA,r6
b3skip:

	/*----------------------------------------------------------- */
	/* Set RTR */
	/*----------------------------------------------------------- */
	cmpi	0, r30, 1600
	bge	rtr_1
	addis   r7, 0, 0x05F0	/* RTR value for 100Mhz */
	bl	rtr_2
rtr_1:	addis	r7, 0, 0x03F8
rtr_2:	addi    r4,0,SDRAM0_RTR
	mtdcr   SDRAM0_CFGADDR,r4
	mtdcr   SDRAM0_CFGDATA,r7

	/*----------------------------------------------------------- */
	/* Delay to ensure 200usec have elapsed since reset. Assume worst */
	/* case that the core is running 200Mhz: */
	/*   200,000,000 (cycles/sec) X .000200 (sec) = 0x9C40 cycles */
	/*----------------------------------------------------------- */
	addis   r3,0,0x0000
	ori     r3,r3,0xA000          /* ensure 200usec have passed since reset */
	mtctr   r3
..spinlp2:
	bdnz    ..spinlp2               /* spin loop */

	/*----------------------------------------------------------- */
	/* Set memory controller options reg, MCOPT1. */
	/* Set DC_EN to '1' and BRD_PRF to '01' for 16 byte PLB Burst  */
	/* read/prefetch. */
	/*----------------------------------------------------------- */
	addi    r4,0,SDRAM0_CFG
	mtdcr   SDRAM0_CFGADDR,r4
	addis   r4,0,0x80C0             /* set DC_EN=1 */
	ori     r4,r4,0x0000
	mtdcr   SDRAM0_CFGDATA,r4


	/*----------------------------------------------------------- */
	/* Delay to ensure 10msec have elapsed since reset. This is */
	/* required for the MPC952 to stabalize. Assume worst */
	/* case that the core is running 200Mhz: */
	/*   200,000,000 (cycles/sec) X .010 (sec) = 0x1E8480 cycles */
	/* This delay should occur before accessing SDRAM. */
	/*----------------------------------------------------------- */
	addis   r3,0,0x001E
	ori     r3,r3,0x8480          /* ensure 10msec have passed since reset */
	mtctr   r3
..spinlp3:
	bdnz    ..spinlp3                /* spin loop */

	/* output SDRAM code  on LEDs */
	addi	r4, 0, LED_SDRAM_CODE_16
	addis	r5, 0, 0x1000
	ori	r5, r5, 0x0001
	stb	r4,0(r5)
	eieio

	mtlr    r31                     /* restore lr */
	blr

/*--------------------------------------------------------------------- */
/* Function:    read_spd */
/* Description: Reads contents of SPD and saves parameters to be used for */
/*		configuration in dedicated registers (see code below). */
/*---------------------------------------------------------------------	 */

#define WRITE_I2C(reg,val) \
	addi    r3,0,val;\
	addis   r4, 0, 0xef60;\
	ori     r4, r4, 0x0500 + reg;\
	stb     r3, 0(r4);\
	eieio

#define READ_I2C(reg) \
	addis   r3, 0, 0xef60;\
	ori     r3, r3, 0x0500 + reg;\
	lbz     r3, 0x0000(r3);\
	eieio

read_spd:

	mflr	r5

	/* Initialize i2c */
	/*--------------- */
	WRITE_I2C(IICLMADR, 0x00)	/* clear lo master address */
	WRITE_I2C(IICHMADR, 0x00)	/* clear hi master address */
	WRITE_I2C(IICLSADR, 0x00)	/* clear lo slave address */
	WRITE_I2C(IICHSADR, 0x00)	/* clear hi slave address */
	WRITE_I2C(IICSTS, 0x08)		/* update status register */
	WRITE_I2C(IICEXTSTS, 0x8f)
	WRITE_I2C(IIC0_CLKDIV, 0x05)
	WRITE_I2C(IICINTRMSK, 0x00)	/* no interrupts */
	WRITE_I2C(IICXFRCNT, 0x00)	/* clear transfer count */
	WRITE_I2C(IICXTCNTLSS, 0xf0)	/* clear extended control & stat */
	WRITE_I2C(IICMDCNTL, IIC_MDCNTL_FSDB | IIC_MDCNTL_FMDB)	/* mode control */
	READ_I2C(IICMDCNTL)
	ori	r3, r3, IIC_MDCNTL_EUBS | IIC_MDCNTL_HSCL
	WRITE_I2C(IICMDCNTL, r3)	/* mode control */
	WRITE_I2C(IICCNTL, 0x00)	/* clear control reg */

	/* Wait until initialization completed */
	/*------------------------------------ */
	bl	wait_i2c_transfer_done

	WRITE_I2C(IICHMADR, 0x00)	/* 7-bit addressing */
	WRITE_I2C(IICLMADR, SDRAM_SPD_WRITE_ADDRESS)

	/* Write 0 into buffer(start address) */
	/*----------------------------------- */
	WRITE_I2C(IICMDBUF, 0x00);

	/* Wait a little */
	/*-------------- */
	addis   r3,0,0x0000
	ori     r3,r3,0xA000
	mtctr   r3
in02:	bdnz    in02

	/* Issue write command */
	/*-------------------- */
	WRITE_I2C(IICCNTL, IIC_CNTL_PT)
	bl	wait_i2c_transfer_done

	/* Read 128 bytes */
	/*--------------- */
	addi	r7, 0, 0	/* byte counter in r7 */
	addi	r8, 0, 0	/* checksum in r8 */
rdlp:
	/* issue read command */
	/*------------------- */
	cmpi	0, r7, 127
	blt	rd01
	WRITE_I2C(IICCNTL, IIC_CNTL_READ | IIC_CNTL_PT)
	bl	rd02
rd01:	WRITE_I2C(IICCNTL, IIC_CNTL_READ | IIC_CNTL_CHT | IIC_CNTL_PT)
rd02:	bl	wait_i2c_transfer_done

	/* Fetch byte from buffer */
	/*----------------------- */
	READ_I2C(IICMDBUF)

	/* Retrieve parameters that are going to be used during configuration. */
	/* Save them in dedicated registers. */
	/*------------------------------------------------------------ */
	cmpi	0, r7, 3	/* Save byte 3 in r10 */
	bne	rd10
	addi	r10, r3, 0
rd10:	cmpi	0, r7, 4	/* Save byte 4 in r11 */
	bne	rd11
	addi	r11, r3, 0
rd11:	cmpi	0, r7, 5	/* Save byte 5 in r12 */
	bne	rd12
	addi	r12, r3, 0
rd12:	cmpi	0, r7, 17	/* Save byte 17 in r13 */
	bne	rd13
	addi	r13, r3, 0
rd13:	cmpi	0, r7, 18	/* Save byte 18 in r14 */
	bne	rd14
	addi	r14, r3, 0
rd14:	cmpi	0, r7, 31	/* Save byte 31 in r15 */
	bne	rd15
	addi	r15, r3, 0
rd15:	cmpi	0, r7, 27	/* Save byte 27 in r16 */
	bne	rd16
	addi	r16, r3, 0
rd16:	cmpi	0, r7, 29	/* Save byte 29 in r17 */
	bne	rd17
	addi	r17, r3, 0
rd17:	cmpi	0, r7, 30	/* Save byte 30 in r18 */
	bne	rd18
	addi	r18, r3, 0
rd18:	cmpi	0, r7, 9	/* Save byte 9 in r19 */
	bne	rd19
	addi	r19, r3, 0
rd19:	cmpi	0, r7, 23	/* Save byte 23 in r20 */
	bne	rd20
	addi	r20, r3, 0
rd20:	cmpi	0, r7, 25	/* Save byte 25 in r21 */
	bne	rd21
	addi	r21, r3, 0
rd21:

	/* Calculate checksum of the first 63 bytes */
	/*----------------------------------------- */
	cmpi	0, r7, 63
	bgt	rd31
	beq	rd30
	add	r8, r8, r3
	bl	rd31

	/* Verify checksum at byte 63 */
	/*--------------------------- */
rd30:	andi.	r8, r8, 0xff		/* use only 8 bits */
	cmp	0, r8, r3
	beq	rd31
	addi	r4, 0, LED_SDRAM_CODE_8
	addis	r5, 0, 0x1000
	ori	r5, r5, 0x0001
	stb	r4,0(r5)
	eieio
rderr:	bl	rderr

rd31:

	/* Increment byte counter and check whether all bytes have been read. */
	/*------------------------------------------------------------------- */
	addi	r7, r7, 1
	cmpi	0, r7, 127
	bgt	rd05
	bl	rdlp
rd05:
	mtlr    r5                     /* restore lr */
	blr

wait_i2c_transfer_done:
	mflr	r6
wt01:	READ_I2C(IICSTS)
	andi.	r4, r3, IIC_STS_PT
	cmpi	0, r4, IIC_STS_PT
	beq	wt01
	mtlr    r6                     /* restore lr */
	blr

/*--------------------------------------------------------------------- */
/* Function:    find_mode */
/* Description: Determines addressing mode to be used dependent on   */
/*		number of rows (r10 = byte 3 from SPD), number of columns (r11 = */
/*		byte 4 from SPD) and number of banks (r13 = byte 17 from SPD). */
/*		mode is returned in r3. */
/* (It would be nicer having a table, pnc). */
/*---------------------------------------------------------------------	 */
find_mode:

	mflr	r5

	cmpi	0, r10, 11
	bne	fm01
	cmpi	0, r11, 9
	bne	fm01
	cmpi	0, r13, 2
	bne	fm01
	addi	r3, 0, 1
	bl	fmfound

fm01:	cmpi	0, r10, 11
	bne	fm02
	cmpi	0, r11, 10
	bne	fm02
	cmpi	0, r13, 2
	bne	fm02
	addi	r3, 0, 1
	bl	fmfound

fm02:	cmpi	0, r10, 12
	bne	fm03
	cmpi	0, r11, 9
	bne	fm03
	cmpi	0, r13, 4
	bne	fm03
	addi	r3, 0, 2
	bl	fmfound

fm03:	cmpi	0, r10, 12
	bne	fm04
	cmpi	0, r11, 10
	bne	fm04
	cmpi	0, r13, 4
	bne	fm04
	addi	r3, 0, 2
	bl	fmfound

fm04:	cmpi	0, r10, 13
	bne	fm05
	cmpi	0, r11, 9
	bne	fm05
	cmpi	0, r13, 4
	bne	fm05
	addi	r3, 0, 3
	bl	fmfound

fm05:	cmpi	0, r10, 13
	bne	fm06
	cmpi	0, r11, 10
	bne	fm06
	cmpi	0, r13, 4
	bne	fm06
	addi	r3, 0, 3
	bl	fmfound

fm06:	cmpi	0, r10, 13
	bne	fm07
	cmpi	0, r11, 11
	bne	fm07
	cmpi	0, r13, 4
	bne	fm07
	addi	r3, 0, 3
	bl	fmfound

fm07:	cmpi	0, r10, 12
	bne	fm08
	cmpi	0, r11, 8
	bne	fm08
	cmpi	0, r13, 2
	bne	fm08
	addi	r3, 0, 4
	bl	fmfound

fm08:	cmpi	0, r10, 12
	bne	fm09
	cmpi	0, r11, 8
	bne	fm09
	cmpi	0, r13, 4
	bne	fm09
	addi	r3, 0, 4
	bl	fmfound

fm09:	cmpi	0, r10, 11
	bne	fm10
	cmpi	0, r11, 8
	bne	fm10
	cmpi	0, r13, 2
	bne	fm10
	addi	r3, 0, 5
	bl	fmfound

fm10:	cmpi	0, r10, 11
	bne	fm11
	cmpi	0, r11, 8
	bne	fm11
	cmpi	0, r13, 4
	bne	fm11
	addi	r3, 0, 5
	bl	fmfound

fm11:	cmpi	0, r10, 13
	bne	fm12
	cmpi	0, r11, 8
	bne	fm12
	cmpi	0, r13, 2
	bne	fm12
	addi	r3, 0, 6
	bl	fmfound

fm12:	cmpi	0, r10, 13
	bne	fm13
	cmpi	0, r11, 8
	bne	fm13
	cmpi	0, r13, 4
	bne	fm13
	addi	r3, 0, 6
	bl	fmfound

fm13:	cmpi	0, r10, 13
	bne	fm14
	cmpi	0, r11, 9
	bne	fm14
	cmpi	0, r13, 2
	bne	fm14
	addi	r3, 0, 7
	bl	fmfound

fm14:	cmpi	0, r10, 13
	bne	fm15
	cmpi	0, r11, 10
	bne	fm15
	cmpi	0, r13, 2
	bne	fm15
	addi	r3, 0, 7
	bl	fmfound

fm15:
	/* not found, error code to be issued on LEDs */
	addi	r7, 0, LED_SDRAM_CODE_2
	addis	r6, 0, 0x1000
	ori	r6, r6, 0x0001
	stb	r7,0(r6)
	eieio
fmerr:	bl	fmerr

fmfound:addi	r6, 0, 1
	subf	r3, r6, r3

	mtlr    r5                     /* restore lr */
	blr

/*--------------------------------------------------------------------- */
/* Function:    find_size_code */
/* Description: Determines size code to be used in configuring SDRAM controller */
/*		dependent on density (r15 = byte 31 from SPD) */
/*--------------------------------------------------------------------- */
find_size_code:

	mflr	r5

	addi	r3, r15, 0	/* density */
	addi	r7, 0, 0
fs01:	andi.	r6, r3, 0x01
	cmpi	0, r6, 1
	beq	fs04

	addi	r7, r7, 1
	cmpi	0, r7, 7
	bge	fs02
	addi	r9, 0, 1
	srw	r3, r3, r9
	bl	fs01

	/* not found, error code to be issued on LEDs */
fs02:	addi	r4, 0, LED_SDRAM_CODE_3
	addis	r8, 0, 0x1000
	ori	r8, r8, 0x0001
	stb	r4,0(r8)
	eieio
fs03:	bl	fs03

fs04:	addi	r3, r7, 0
	cmpi	0, r3, 0
	beq	fs05
	addi	r6, 0, 1
	subf	r3, r6, r3
fs05:
	mtlr    r5                     /* restore lr */
	blr

/*--------------------------------------------------------------------- */
/* Function:    find_casl */
/* Description: Determines CAS latency */
/*--------------------------------------------------------------------- */
find_casl:

	mflr	r5

	andi.	r14, r14, 0x7f	/* r14 holds supported CAS latencies */
	addi	r3, 0, 0xff	/* preset determined CASL */
	addi	r4, 0, 6	/* Start at bit 6 of supported CAS latencies */
	addi	r2, 0, 0	/* Start finding highest CAS latency */

fc01:	srw	r6, r14, r4	/*  */
	andi.	r6, r6, 0x01	/*  */
	cmpi	0, r6, 1	/* Check bit for current latency */
	bne	fc06		/* If not supported, go to next */

	cmpi	0, r2, 2	/* Check if third-highest latency */
	bge	fc04		/* If so, go calculate with another format */

	cmpi	0, r2, 0	/* Check if highest latency */
	bgt	fc02		/* */
	addi	r7, r19, 0	/* SDRAM cycle time for highest CAS latenty */

	bl	fc03
fc02:
	addi	r7, r20, 0	/* SDRAM cycle time for next-highest CAS latenty */
fc03:
	addi	r8, r7, 0
	addi	r9, 0, 4
	srw	r7, r7, r9
	andi.	r7, r7, 0x0f
	mulli	r7, r7, 100
	andi.	r8, r8, 0x0f
	mulli	r8, r8, 10
	add	r7, r7, r8
	cmp	0, r7, r30
	bgt	fc05
	addi	r3, r2, 0
	bl	fc05
fc04:
	addi	r7, r21, 0	/* SDRAM cycle time for third-highest CAS latenty */
	addi	r8, r7, 0
	addi	r9, 0, 2
	srw	r7, r7, r9
	andi.	r7, r7, 0x3f
	mulli	r7, r7, 100
	andi.	r8, r8, 0x03
	mulli	r8, r8, 25
	add	r7, r7, r8

	cmp	0, r7, r30
	bgt	fc05
	addi	r3, r2, 0

fc05:	addi	r2, r2, 1	/* next latency */
	cmpi	0, r2, 3
	bge	fc07
fc06:	addi	r6, 0, 1
	subf	r4, r6, r4
	cmpi	0, r4, 0
	bne	fc01

fc07:

	mtlr    r5		/* restore lr */
	blr
#endif


/*  Peripheral Bank 1 Access Parameters */
/*     0	BME = 1	; burstmode enabled */
/*    " 1:8"	TWT=00110110	;Transfer wait (details below) */
/*     1:5	FWT=00110	; first wait = 6 cycles */
/*     6:8	BWT=110	; burst wait = 6 cycles */
/*     9:11	000	; reserved */
/*     12:13	CSN=00	; chip select on timing = 0 */
/*     14:15	OEN=01	; output enable  */
/*     16:17	WBN=01	; write byte enable on timing 1 cycle */
/*     18:19	WBF=01	; write byte enable off timing 1 cycle */
/*     20:22	TH=010	; transfer hold = 2 cycles */
/*     23	RE=0	; ready enable = disabled */
/*     24	SOR=1	; sample on ready = same PerClk */
/*     25	BEM=0	; byte enable mode = only for write cycles */
/*     26	PEN=0	; parity enable = disable */
/*     27:31	00000	;reserved */
/* */
/* 1 + 00110 + 110 + 000 + 00 + 01 + 01 + 01 + 010 + 0 + 1 + 0 + 0 + 00000 = 0x9b015480 */
/* */
/* */
/*	Code for BDI probe: */
/* */
/* WDCR    18      0x00000011      ;Select PB1AP */
/* WDCR    19      0x1b015480      ;PB1AP: Flash */
/* */
/* Peripheral Bank 0 Access Parameters */
/* 0:11	BAS=0x200	; base address select = 0x200 * 0x100000 (1MB) =  */
/* 12:14	BS=100	; bank size =  16MB (100) / 32MB (101) */
/* 15:16	BU=11	; bank usage = read/write */
/* 17:18	BW=00	; bus width = 8-bit */
/* 19:31		; reserved */
/* */
/* 0x200 + 100 + 11 + 00 + 0 0000 0000 0000 = 0x20098000 */
/* WDCR    18      0x00000001      ;Select PB1CR */
/* WDCR    19      0x20098000      ;PB1CR: 1MB at 0x00100000, r/w, 8bit */

/* For CPLD */
/* 0 + 00000 + 010 + 000 + 00 + 01 + 00 + 00 + 000 + 0 + 0 + 1 + 0 + 00000 */
/*	WDCR_EBC(PB5AP, 0x01010040) */
/*jsa recommendation:		WDCR_EBC(PB5AP, 0x00010040) */
/*	WDCR_EBC(PB5CR, 0X10018000) */
/* Access parms */
/*   100   3      8          0    0    0 */
/* 0x100 + 001 + 11 + 00 + 0 0000 0000 0000 = 0x10038000 */
/* Address :	0x10000000 */
/* Size:	2 MB */
/* Usage:	read/write */
/* Width:	32 bit */

/* For Genie onboard fpga 32 bit interface */
/* 0      1      0         1         0         0         0            0 */
/* 0 + 00000 + 010 + 000 + 00 + 01 + 00 + 00 + 000 + 0 + 0 + 0 + 0 + 00000 */
/* 0x01010000 */
/* Access parms */
/*   102   1      c          0    0    0 */
/* 0x102 + 000 + 11 + 10 + 0 0000 0000 0000 = 0x1021c000 */
/* Address :	0x10200000 */
/* Size:	2 MB */
/* Usage:	read/write */
/* Width:	32 bit */

/* Walnut fpga PB7AP */
/* 0      1      8         1         5         2         8            0 */
/* 0 + 00000 + 011 + 000 + 00 + 01 + 01 + 01 + 001 + 0 + 1 + 0 + 0 + 00000 */
/* Walnut fpga PB7CR */
/* 0xF0318000 */
/*  */
