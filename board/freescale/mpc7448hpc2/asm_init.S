/*
 * (C) Copyright 2004-05;  Tundra Semiconductor Corp.
 *
 * Added automatic detect of SDC settings
 * Copyright (c) 2005 Freescale Semiconductor, Inc.
 * Maintainer tie-fei.zang@freescale.com
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

/*
 * FILENAME: asm_init.s
 *
 * Originator: Alex Bounine
 *
 * DESCRIPTION:
 * Initialization code for the Tundra Tsi108 bridge chip
 *
 */

#include <config.h>
#include <version.h>

#include <ppc_asm.tmpl>
#include <ppc_defs.h>
#include <asm/processor.h>

#include <tsi108.h>

/*
 * Build Configuration Options
 */

/* #define DISABLE_PBM		disables usage of PB Master */
/* #define SDC_HARDCODED_INIT	config SDRAM controller with hardcoded values */
/* #define SDC_AUTOPRECH_EN	enable SDRAM auto precharge */

/*
 * Hardcoded SDC settings
 */

#ifdef SDC_HARDCODED_INIT

/* Micron MT9HTF6472AY-40EA1 : Unbuffered, 512MB, 400, CL3, Single Rank */

#define VAL_SD_REFRESH	(0x61A)
#define VAL_SD_TIMING	(0x0308336b)
#define VAL_SD_D0_CTRL	(0x07100021)	/* auto-precharge disabled */
#define VAL_SD_D0_BAR	(0x0FE00000)	/* 512MB @ 0x00000000 */
#define VAL_SD_D1_CTRL	(0x07100021)	/* auto-precharge disabled */
#define VAL_SD_D1_BAR	(0x0FE00200)	/* 512MB @ 0x20000000 */

#endif /* SDC_HARDCODED_INIT */

/*
 CPU Configuration:

 CPU Address and Data Parity enables.

#define CPU_AP
#define CPU_DP
*/

/*
 * Macros
 * !!! Attention !!! Macros LOAD_PTR, LOAD_U32 and LOAD_MEM defined below are
 * expected to work correctly for the CSR space within 32KB range.
 *
 * LOAD_PTR and LOAD_U32 - load specified register with a 32 bit constant.
 * These macros are absolutely identical except their names. This difference
 * is provided intentionally for better readable code.
 */

#define LOAD_PTR(reg,const32) \
	addis reg,r0,const32@h; ori reg,reg,const32@l

#define LOAD_U32(reg,const32) \
	addis reg,r0,const32@h; ori reg,reg,const32@l

/* LOADMEM initializes a register with the contents of a specified 32-bit
 * memory location, usually a CSR value.
 */

#define LOAD_MEM(reg,addr32) \
	addis reg,r0,addr32@ha; lwz reg,addr32@l(reg)

#ifndef SDC_HARDCODED_INIT
sdc_clk_sync:
	/* MHz: 0,0,183,100,133,167,200,233 */
	.long	0, 0, 6, 10, 8, 6, 5, 4		/* nSec */
#endif

/*
 * board_asm_init() - early initialization function. Coded to be portable to
 * dual-CPU configuration.
 * Checks CPU number and performs board HW initialization if called for CPU0.
 * Registers used: r3,r4,r5,r6,r19,r29
 *
 * NOTE: For dual-CPU configuration only CPU0 is allowed to configure Tsi108
 * and the rest of the board. Current implementation demonstrates two
 * possible ways to identify CPU number:
 * - for MPC74xx platform: uses MSSCR0[ID] bit as defined in UM.
 * - for PPC750FX/GX boards: uses WHO_AM_I bit reported by Tsi108.
 */

	.globl board_asm_init
board_asm_init:
	mflr	r19	/* Save LR to be able return later. */
	bl	icache_enable	/* Enable icache to reduce reads from flash. */

/* Initialize pointer to Tsi108 register space */

	LOAD_PTR(r29,CFG_TSI108_CSR_RST_BASE)/* r29 - pointer to tsi108 CSR space */
	ori r4,r29,TSI108_PB_REG_OFFSET

/* Check Processor Version Number */

	mfspr	r3, PVR
	rlwinm	r3,r3,16,16,23	/* get ((Processor Version Number) & 0xFF00) */

	cmpli	0,0,r3,0x8000	/* MPC74xx */
	bne	cont_brd_init

	/*
	 * For MPC744x/5x enable extended BATs[4-7]
	 * Sri: Set HIGH_BAT_EN and XBSEN, and SPD =1
	 * to disable prefetch
	 */

	mfspr	r5, HID0
	oris	r5, r5, 0x0080	/* Set HID0[HIGH_BAT_EN] bit #8 */
	ori	r5, r5, 0x0380	/* Set SPD,XBSEN,SGE bits #22,23,24 */
	mtspr	HID0, r5
	isync
	sync

	/* Adding code to disable external interventions in MPX bus mode */
	mfspr	r3, 1014
	oris	r3, r3, 0x0100	/* Set the EIDIS bit in MSSCR0:  bit 7 */
	mtspr	1014, r3
	isync
	sync

	/* Sri: code to enable FP unit */
	mfmsr	r3
	ori	r3, r3, 0x2000
	mtmsr	r3
	isync
	sync

	/* def CONFIG_DUAL_CPU
	 * For MPC74xx processor, use MSSCR0[ID] bit to identify CPU number.
	 */
#if(1)
	mfspr	r3,1014		/* read MSSCR0 */
	rlwinm.	r3,r3,27,31,31	/* get processor ID number */
	mtspr	SPRN_PIR,r3	/* Save CPU ID */
	sync
	bne	init_done
	b	do_tsi108_init

cont_brd_init:

	/* An alternative method of checking the processor number (in addition
	 * to configuration using MSSCR0[ID] bit on MPC74xx).
	 * Good for IBM PPC750FX/GX.
	 */

	lwz	r3,PB_BUS_MS_SELECT(r4)	/* read PB_ID register */
	rlwinm.	r3,r3,24,31,31		/* get processor ID number */
	bne init_done
#else

cont_brd_init:

#endif /* CONFIG_DUAL_CPU */

	/* Initialize Tsi108 chip */

do_tsi108_init:

	/*
	 * Adjust HLP/Flash parameters. By default after reset the HLP port is
	 * set to support slow devices. Better performance can be achived when
	 * an optimal parameters are used for specific EPROM device.
	 * NOTE: This should be performed ASAP for the emulation platform
	 * because it has 5MHz HLP clocking.
	 */

#ifdef CONFIG_TSI108EMU
	ori	r4,r29,TSI108_HLP_REG_OFFSET
	LOAD_U32(r5,0x434422c0)
	stw	r5,0x08(r4)	/* set HLP B0_CTRL0 */
	sync
	LOAD_U32(r5,0xd0012000)
	stw	r5,0x0c(r4)		/* set HLP B0_CTRL1 */
	sync
#endif

	/* Initialize PB interface. */

	ori r4,r29,TSI108_PB_REG_OFFSET

#if (CFG_TSI108_CSR_BASE != CFG_TSI108_CSR_RST_BASE)
	/* Relocate (if required) Tsi108 registers. Set new value for
	 * PB_REG_BAR:
	 * Note we are in the 32-bit address mode.
	 */
	LOAD_U32(r5,(CFG_TSI108_CSR_BASE | 0x01)) /* PB_REG_BAR: BA + EN */
	stw	r5,PB_REG_BAR(r4)
	andis.	r29,r5,0xFFFF
	sync
	ori	r4,r29,TSI108_PB_REG_OFFSET
#endif

	/* Set PB Slave configuration register */

	LOAD_U32(r5,0x00002481)	/* PB_SCR: TEA enabled,AACK delay = 1 */
	lwz	r3, PB_RSR(r4)	/* get PB bus mode */
	xori	r3,r3,0x0001	/* mask PB_BMODE: r3 -> (0 = 60X, 1 = MPX) */
	rlwimi  r5,r3,14,17,17	/* for MPX: set DTI_MODE bit */
	stw	r5,PB_SCR(r4)
	sync

	/* Configure PB Arbiter */

	lwz	r5,PB_ARB_CTRL(r4)	/* Read PB Arbiter Control Register */
	li	r3, 0x00F0		/* ARB_PIPELINE_DEP mask */
#ifdef DISABLE_PBM
	ori	r3,r3,0x1000	/* add PBM_EN to clear (enabled by default) */
#endif
	andc	r5,r5,r3	/* Clear the masked bit fields */
	ori	r5,r5,0x0001	/* Set pipeline depth */
	stw	r5,PB_ARB_CTRL(r4)

#if (0)	/* currently using the default settings for PBM after reset */
	LOAD_U32(r5,0x)		/* value for PB_MCR */
	stw	r5,PB_MCR(r4)
	sync

	LOAD_U32(r5,0x)		/* value for PB_MCMD */
	stw	r5,PB_MCMD(r4)
	sync
#endif

	/* Disable or enable PVT based on processor bus frequency
	 * 1. Read CG_PWRUP_STATUS register field bits 18,17,16
	 * 2. See if the value is < or > 133mhz (18:16 = 100)
	 * 3. If > enable PVT
	 */

	LOAD_U32(r3,0xC0002234)
	lwz	r3,0(r3)
	rlwinm	r3,r3,16,29,31

	cmpi	0,0,r3,0x0004
	bgt	sdc_init

#ifndef CONFIG_TSI108EMU
	/* FIXME: Disable PB calibration control for any real Tsi108 board */
	li	r5,0x0101	/* disable calibration control */
	stw	r5,PB_PVT_CTRL2(r4)
	sync
#endif

	/* Initialize SDRAM controller. */

sdc_init:

#ifndef SDC_HARDCODED_INIT
	/* get SDC clock prior doing sdram controller autoconfig */
	ori	r4,r29,TSI108_CLK_REG_OFFSET	/* r4 - ptr to CG registers */
	lwz	r3, CG_PWRUP_STATUS(r4)		/* get CG configuration */
	rlwinm	r3,r3,12,29,31			/* r3 - SD clk */
	lis	r5,sdc_clk_sync@h
	ori	r5,r5,sdc_clk_sync@l
	/* Sri:  At this point check if r3 = 001. If yes,
	 * the memory frequency should be same as the
	 * MPX bus frequency
	 */
	cmpi	0,0,r3,0x0001
	bne	get_nsec
	lwz	r6, CG_PWRUP_STATUS(r4)
	rlwinm	r6,r6,16,29,31
	mr	r3,r6

get_nsec:
	rlwinm	r3,r3,2,0,31
	lwzx	r9,r5,r3	/* get SD clk rate in nSec */
	/* ATTN: r9 will be used by SPD routine */
#endif /* !SDC_HARDCODED_INIT */

	ori	r4,r29,TSI108_SD_REG_OFFSET /* r4 - ptr to SDRAM registers */

	/* Initialize SDRAM controller. SDRAM Size = 512MB, One DIMM. */

	LOAD_U32(r5,0x00)
	stw	r5,SD_INT_ENABLE(r4) /* Ensure that interrupts are disabled */
#ifdef ENABLE_SDRAM_ECC
	li	r5, 0x01
#endif /* ENABLE_SDRAM_ECC */
	stw	r5,SD_ECC_CTRL(r4)	/* Enable/Disable ECC */
	sync

#ifdef SDC_HARDCODED_INIT /* config sdram controller with hardcoded values */

	/* First read the CG_PWRUP_STATUS register to get the
	 * memory speed from bits 22,21,20
	 */

	LOAD_U32(r3,0xC0002234)
	lwz	r3,0(r3)
	rlwinm	r3,r3,12,29,31

	/* Now first check for 166, then 200, or default */

	cmpi	0,0,r3,0x0005
	bne	check_for_200mhz

	/* set values for 166 Mhz memory speed
	 * Set refresh rate and timing parameters
	 */
	LOAD_U32(r5,0x00000515)
	stw	r5,SD_REFRESH(r4)
	LOAD_U32(r5,0x03073368)
	stw	r5,SD_TIMING(r4)
	sync

	/* Initialize DIMM0 control and BAR registers */
	LOAD_U32(r5,VAL_SD_D0_CTRL)	/* auto-precharge disabled */
#ifdef SDC_AUTOPRECH_EN
	oris	r5,r5,0x0001		/* set auto precharge EN bit */
#endif
	stw	r5,SD_D0_CTRL(r4)
	LOAD_U32(r5,VAL_SD_D0_BAR)
	stw	r5,SD_D0_BAR(r4)
	sync

	/* Initialize DIMM1 control and BAR registers
	 * (same as dimm 0, next 512MB, disabled)
	 */
	LOAD_U32(r5,VAL_SD_D1_CTRL)	/* auto-precharge disabled */
#ifdef SDC_AUTOPRECH_EN
	oris	r5,r5,0x0001	/* set auto precharge EN bit */
#endif
	stw	r5,SD_D1_CTRL(r4)
	LOAD_U32(r5,VAL_SD_D1_BAR)
	stw	r5,SD_D1_BAR(r4)
	sync

	b	sdc_init_done

check_for_200mhz:

	cmpi	0,0,r3,0x0006
	bne	set_default_values

	/* set values for 200Mhz memory speed
	 * Set refresh rate and timing parameters
	 */
	LOAD_U32(r5,0x0000061a)
	stw	r5,SD_REFRESH(r4)
	LOAD_U32(r5,0x03083348)
	stw	r5,SD_TIMING(r4)
	sync

	/* Initialize DIMM0 control and BAR registers */
	LOAD_U32(r5,VAL_SD_D0_CTRL)	/* auto-precharge disabled */
#ifdef SDC_AUTOPRECH_EN
	oris	r5,r5,0x0001		/* set auto precharge EN bit */
#endif
	stw	r5,SD_D0_CTRL(r4)
	LOAD_U32(r5,VAL_SD_D0_BAR)
	stw	r5,SD_D0_BAR(r4)
	sync

	/* Initialize DIMM1 control and BAR registers
	 * (same as dimm 0, next 512MB, disabled)
	 */
	LOAD_U32(r5,VAL_SD_D1_CTRL)	/* auto-precharge disabled */
#ifdef SDC_AUTOPRECH_EN
	oris	r5,r5,0x0001		/* set auto precharge EN bit */
#endif
	stw	r5,SD_D1_CTRL(r4)
	LOAD_U32(r5,VAL_SD_D1_BAR)
	stw	r5,SD_D1_BAR(r4)
	sync

	b	sdc_init_done

set_default_values:

	/* Set refresh rate and timing parameters */
	LOAD_U32(r5,VAL_SD_REFRESH)
	stw	r5,SD_REFRESH(r4)
	LOAD_U32(r5,VAL_SD_TIMING)
	stw	r5,SD_TIMING(r4)
	sync

	/* Initialize DIMM0 control and BAR registers */
	LOAD_U32(r5,VAL_SD_D0_CTRL)	/* auto-precharge disabled */
#ifdef SDC_AUTOPRECH_EN
	oris	r5,r5,0x0001		/* set auto precharge EN bit */
#endif
	stw	r5,SD_D0_CTRL(r4)
	LOAD_U32(r5,VAL_SD_D0_BAR)
	stw	r5,SD_D0_BAR(r4)
	sync

	/* Initialize DIMM1 control and BAR registers
	 * (same as dimm 0, next 512MB, disabled)
	 */
	LOAD_U32(r5,VAL_SD_D1_CTRL)	/* auto-precharge disabled */
#ifdef SDC_AUTOPRECH_EN
	oris	r5,r5,0x0001		/* set auto precharge EN bit */
#endif
	stw	r5,SD_D1_CTRL(r4)
	LOAD_U32(r5,VAL_SD_D1_BAR)
	stw	r5,SD_D1_BAR(r4)
	sync
#else /* !SDC_HARDCODED_INIT */
	bl	tsi108_sdram_spd	/* automatically detect SDC settings */
#endif /* SDC_HARDCODED_INIT */

sdc_init_done:

#ifdef DISABLE_PBM
	LOAD_U32(r5,0x00000030)		/* PB_EN + OCN_EN */
#else
	LOAD_U32(r5,0x00000230)		/* PB_EN + OCN_EN + PB/OCN=80/20 */
#endif /* DISABLE_PBM */

#ifdef CONFIG_TSI108EMU
	oris	r5,r5,0x0010		/* set EMULATION_MODE bit */
#endif

	stw	r5,SD_CTRL(r4)
	eieio
	sync

	/* Enable SDRAM access */

	oris	r5,r5,0x8000		/* start SDC: set SD_CTRL[ENABLE] bit */
	stw	r5,SD_CTRL(r4)
	sync

wait_init_complete:
	lwz	r5,SD_STATUS(r4)
	andi.	r5,r5,0x0001
	/* wait until SDRAM initialization is complete */
	beq	wait_init_complete

	/* Map SDRAM into the processor bus address space */

	ori	r4,r29,TSI108_PB_REG_OFFSET

	/* Setup BARs associated with direct path PB<->SDRAM */

	/* PB_SDRAM_BAR1:
	 * provides a direct path to the main system memory (cacheable SDRAM)
	 */

	/* BA=0,Size=512MB, ENable, No Addr.Translation */
	LOAD_U32(r5, 0x00000011)
	stw	r5,PB_SDRAM_BAR1(r4)
	sync

	/* Make sure that PB_SDRAM_BAR1 decoder is set
	 * (to allow following immediate read from SDRAM)
	 */
	lwz	r5,PB_SDRAM_BAR1(r4)
	sync

	/* PB_SDRAM_BAR2:
	 * provides non-cacheable alias (via the direct path) to main
	 * system memory.
	 * Size = 512MB, ENable, Addr.Translation - ON,
	 * BA = 0x0_40000000, TA = 0x0_00000000
	 */

	LOAD_U32(r5, 0x40010011)
	stw	r5,PB_SDRAM_BAR2(r4)
	sync

	/* Make sure that PB_SDRAM_BAR2 decoder is set
	 * (to allow following immediate read from SDRAM)
	 */
	lwz	r5,PB_SDRAM_BAR2(r4)
	sync

init_done:

	/* All done. Restore LR and return. */
	mtlr	r19
	blr

#if (0)
	/*
	 * init_cpu1
	 * This routine enables CPU1 on the dual-processor system.
	 * Now there is only one processor in the system
	 */

	.global enable_cpu1
enable_cpu1:

	lis	r3,Tsi108_Base@ha	/* Get Grendel CSR Base Addr */
	addi	r3,r3,Tsi108_Base@l
	lwz	r3,0(r3)		/* R3 = CSR Base Addr */
	ori	r4,r3,TSI108_PB_REG_OFFSET
	lwz	r3,PB_ARB_CTRL(r4)	/* Read PB Arbiter Control Register */
	ori	r3,r3,0x0200		/* Set M1_EN bit */
	stw	r3,PB_ARB_CTRL(r4)

	blr
#endif

	/*
	 * enable_EI
	 * Enable CPU core external interrupt
	 */

	.global	enable_EI
enable_EI:
	mfmsr	r3
	ori	r3,r3,0x8000	/* set EE bit */
	mtmsr	r3
	blr

	/*
	 * disable_EI
	 * Disable CPU core external interrupt
	 */

	.global disable_EI
disable_EI:
	mfmsr	r3
	li	r4,-32768	/* aka "li  r4,0x8000" */
	andc	r3,r3,r4	/* clear EE bit */
	mtmsr	r3
	blr

#ifdef ENABLE_SDRAM_ECC
	/* enables SDRAM ECC  */

	.global	enable_ECC
enable_ECC:
	ori	r4,r29,TSI108_SD_REG_OFFSET
	lwz	r3,SD_ECC_CTRL(r4)	/* Read SDRAM ECC Control Register */
	ori	r3,r3,0x0001		/* Set ECC_EN bit */
	stw	r3,SD_ECC_CTRL(r4)
	blr

	/*
	 * clear_ECC_err
	 * Clears all pending SDRAM ECC errors
	 * (normally after SDRAM scrubbing/initialization)
	 */

	.global	clear_ECC_err
clear_ECC_err:
	ori r4,r29,TSI108_SD_REG_OFFSET
	ori r3,r0,0x0030	/* ECC_UE_INT + ECC_CE_INT bits */
	stw r3,SD_INT_STATUS(r4)
	blr

#endif /* ENABLE_SDRAM_ECC */

#ifndef SDC_HARDCODED_INIT

	/* SDRAM SPD Support */
#define	SD_I2C_CTRL1	(0x400)
#define	SD_I2C_CTRL2	(0x404)
#define SD_I2C_RD_DATA	(0x408)
#define SD_I2C_WR_DATA	(0x40C)

	/*
	 * SDRAM SPD Support Macros
	 */

#define SPD_DIMM0	(0x00000100)
#define SPD_DIMM1	(0x00000200)	/* SPD_DIMM1 was 0x00000000 */

#define SPD_RDIMM			(0x01)
#define SPD_UDIMM			(0x02)

#define SPD_CAS_3			0x8
#define SPD_CAS_4			0x10
#define SPD_CAS_5			0x20

#define ERR_NO_DIMM_FOUND		(0xdb0)
#define ERR_TRAS_FAIL			(0xdb1)
#define ERR_TRCD_FAIL			(0xdb2)
#define ERR_TRP_FAIL			(0xdb3)
#define ERR_TWR_FAIL			(0xdb4)
#define ERR_UNKNOWN_PART		(0xdb5)
#define ERR_NRANK_INVALID		(0xdb6)
#define ERR_DIMM_SIZE			(0xdb7)
#define ERR_ADDR_MODE			(0xdb8)
#define ERR_RFRSH_RATE			(0xdb9)
#define ERR_DIMM_TYPE			(0xdba)
#define ERR_CL_VALUE			(0xdbb)
#define ERR_TRFC_FAIL			(0xdbc)

/* READ_SPD requirements:
 * byte - byte address in SPD device (0 - 255)
 * r3 = will return data read from I2C Byte location
 * r4 - unchanged (SDC base addr)
 * r5 - clobbered in routine (I2C status)
 * r10 - number of DDR slot where first SPD device is detected
 */

#define READ_SPD(byte_num)		\
	addis	r3, 0, byte_num@l;	\
	or	r3, r3, r10;		\
	ori	r3, r3, 0x0A;		\
	stw	r3, SD_I2C_CTRL1(r4);	\
	li	r3, I2C_CNTRL2_START;	\
	stw	r3, SD_I2C_CTRL2(r4);	\
	eieio;				\
	sync;				\
	li	r3, 0x100;		\
1:;					\
	addic.	r3, r3, -1;		\
	bne	1b;			\
2:;					\
	lwz	r5, SD_I2C_CTRL2(r4);	\
	rlwinm.	r3,r5,0,23,23;		\
	bne	2b;			\
	rlwinm.	r3,r5,0,3,3;		\
	lwz	r3,SD_I2C_RD_DATA(r4)

#define SPD_MIN_RFRSH	(0x80)
#define SPD_MAX_RFRSH	(0x85)

refresh_rates:	/* in nSec */
	.long	15625	/* Normal (0x80) */
	.long	3900	/* Reduced 0.25x (0x81) */
	.long	7800	/* Reduced 0.5x (0x82) */
	.long	31300	/* Extended 2x (0x83) */
	.long	62500	/* Extended 4x (0x84) */
	.long	125000	/* Extended 8x (0x85) */

/*
 * tsi108_sdram_spd
 *
 * Inittializes SDRAM Controller using DDR2 DIMM Serial Presence Detect data
 * Uses registers: r4 - SDC base address (not changed)
 *				   r9 - SDC clocking period in nSec
 * Changes registers: r3,r5,r6,r7,r8,r10,r11
 */

tsi108_sdram_spd:

	li	r10,SPD_DIMM0
	xor	r11,r11,r11		/* DIMM Base Address: starts from 0 */

do_first_dimm:

	/* Program Refresh Rate	Register */

	READ_SPD(12)			/* get Refresh Rate */
	beq	check_next_slot
	li	r5, ERR_RFRSH_RATE
	cmpi	0,0,r3,SPD_MIN_RFRSH
	ble	spd_fail
	cmpi	0,0,r3,SPD_MAX_RFRSH
	bgt	spd_fail
	addi	r3,r3,-SPD_MIN_RFRSH
	rlwinm	r3,r3,2,0,31
	lis	r5,refresh_rates@h
	ori	r5,r5,refresh_rates@l
	lwzx	r5,r5,r3		/* get refresh rate in nSec */
	divwu	r5,r5,r9		/* calculate # of SDC clocks */
	stw	r5,SD_REFRESH(r4)	/* Set refresh rate */
	sync

	/* Program SD Timing Register */

	li	r7, 0		/* clear r7 prior parameter collection */

	READ_SPD(20)		/* get DIMM type: Registered or Unbuffered */
	beq	spd_read_fail
	li	r5, ERR_DIMM_TYPE
	cmpi	0,0,r3,SPD_UDIMM
	beq	do_cl
	cmpi	0,0,r3,SPD_RDIMM
	bne	spd_fail
	oris	r7,r7,0x1000	/* set SD_TIMING[DIMM_TYPE] bit */

do_cl:
	READ_SPD(18)		/* Get CAS Latency */
	beq	spd_read_fail
	li	r5,ERR_CL_VALUE
	andi.	r6,r3,SPD_CAS_3
	beq	cl_4
	li	r6,3
	b	set_cl
cl_4:
	andi.	r6,r3,SPD_CAS_4
	beq	cl_5
	li	r6,4
	b	set_cl
cl_5:
	andi.	r6,r3,SPD_CAS_5
	beq	spd_fail
	li	r6,5
set_cl:
	rlwimi	r7,r6,24,5,7

	READ_SPD(30)		/* Get tRAS */
	beq	spd_read_fail
	divwu	r6,r3,r9
	mullw	r8,r6,r9
	subf.	r8,r8,r3
	beq	set_tras
	addi	r6,r6,1
set_tras:
	li r5,ERR_TRAS_FAIL
	cmpi	0,0,r6,0x0F	/* max supported value */
	bgt	spd_fail
	rlwimi	r7,r6,16,12,15

	READ_SPD(29)	/* Get tRCD */
	beq	spd_read_fail
	/* right shift tRCD by 2 bits as per DDR2 spec */
	rlwinm	r3,r3,30,2,31
	divwu	r6,r3,r9
	mullw	r8,r6,r9
	subf.	r8,r8,r3
	beq	set_trcd
	addi	r6,r6,1
set_trcd:
	li	r5,ERR_TRCD_FAIL
	cmpi	0,0,r6,0x07	/* max supported value */
	bgt	spd_fail
	rlwimi	r7,r6,12,17,19

	READ_SPD(27)	/* Get tRP value */
	beq	spd_read_fail
	rlwinm	r3,r3,30,2,31	/* right shift tRP by 2 bits as per DDR2 spec */
	divwu	r6,r3,r9
	mullw	r8,r6,r9
	subf.	r8,r8,r3
	beq	set_trp
	addi	r6,r6,1
set_trp:
	li	r5,ERR_TRP_FAIL
	cmpi	0,0,r6,0x07	/* max supported value */
	bgt	spd_fail
	rlwimi	r7,r6,8,21,23

	READ_SPD(36)	/* Get tWR value */
	beq	spd_read_fail
	rlwinm	r3,r3,30,2,31	/* right shift tWR by 2 bits as per DDR2 spec */
	divwu	r6,r3,r9
	mullw	r8,r6,r9
	subf.	r8,r8,r3
	beq	set_twr
	addi	r6,r6,1
set_twr:
	addi	r6,r6,-1	/* Tsi108 SDC always gives one extra clock */
	li	r5,ERR_TWR_FAIL
	cmpi	0,0,r6,0x07	/* max supported value */
	bgt	spd_fail
	rlwimi	r7,r6,5,24,26

	READ_SPD(42)	/* Get tRFC */
	beq	spd_read_fail
	li	r5, ERR_TRFC_FAIL
	/* Tsi108 spec: tRFC=(tRFC + 1)/2 */
	addi	r3,r3,1
	rlwinm.	r3,r3,31,1,31	/* divide by 2 */
	beq	spd_fail
	divwu	r6,r3,r9
	mullw	r8,r6,r9
	subf.	r8,r8,r3
	beq	set_trfc
	addi	r6,r6,1
set_trfc:
	cmpi	0,0,r6,0x1F	/* max supported value */
	bgt	spd_fail
	rlwimi	r7,r6,0,27,31

	stw	r7,SD_TIMING(r4)
	sync

	/*
	 * The following two registers are set on per-DIMM basis.
	 * The SD_REFRESH and SD_TIMING settings are common for both DIMMS
	 */

do_each_dimm:

	/* Program SDRAM DIMM Control Register */

	li	r7, 0		/* clear r7 prior parameter collection */

	READ_SPD(13)		/* Get Primary SDRAM Width */
	beq	spd_read_fail
	cmpi	0,0,r3,4	/* Check for 4-bit SDRAM */
	beq	do_nbank
	oris	r7,r7,0x0010	/* Set MEM_WIDTH bit */

do_nbank:
	READ_SPD(17)		/* Get Number of banks on SDRAM device */
	beq	spd_read_fail
	/* Grendel only distinguish betw. 4 or 8-bank memory parts */
	li	r5,ERR_UNKNOWN_PART	/* non-supported memory part */
	cmpi	0,0,r3,4
	beq	do_nrank
	cmpi	0,0,r3,8
	bne	spd_fail
	ori	r7,r7,0x1000

do_nrank:
	READ_SPD(5)		/* Get # of Ranks */
	beq	spd_read_fail
	li	r5,ERR_NRANK_INVALID
	andi.	r6,r3,0x7	/* Use bits [2..0] only */
	beq	do_addr_mode
	cmpi	0,0,r6,1
	bgt	spd_fail
	rlwimi	r7,r6,8,23,23

do_addr_mode:
	READ_SPD(4)		/* Get # of Column Addresses */
	beq	spd_read_fail
	li	r5, ERR_ADDR_MODE
	andi.	r3,r3,0x0f	/* cut off reserved bits */
	cmpi	0,0,r3,8
	ble	spd_fail
	cmpi	0,0,r3,15
	bgt	spd_fail
	addi	r6,r3,-8	/* calculate ADDR_MODE parameter */
	rlwimi	r7,r6,4,24,27	/* set ADDR_MODE field */

set_dimm_ctrl:
#ifdef SDC_AUTOPRECH_EN
	oris	r7,r7,0x0001	/* set auto precharge EN bit */
#endif
	ori	r7,r7,1		/* set ENABLE bit */
	cmpi	0,0,r10,SPD_DIMM0
	bne	1f
	stw	r7,SD_D0_CTRL(r4)
	sync
	b	set_dimm_bar
1:
	stw	r7,SD_D1_CTRL(r4)
	sync


	/* Program SDRAM DIMMx Base Address Register */

set_dimm_bar:
	READ_SPD(5)		/* get # of Ranks */
	beq	spd_read_fail
	andi.	r7,r3,0x7
	addi	r7,r7,1
	READ_SPD(31)		/* Read DIMM rank density */
	beq	spd_read_fail
	rlwinm	r5,r3,27,29,31
	rlwinm	r6,r3,3,24,28
	or	r5,r6,r5	/* r5 = Normalized Rank Density byte */
	lis	r8, 0x0080	/* 128MB >> 4 */
	mullw	r8,r8,r5	/* r8 = (rank_size >> 4) */
	mullw	r8,r8,r7	/* r8 = (DIMM_size >> 4) */
	neg	r7,r8
	rlwinm	r7,r7,28,4,31
	or	r7,r7,r11	/* set ADDR field */
	rlwinm	r8,r8,12,20,31
	add	r11,r11,r8	/* set Base Addr for next DIMM */

	cmpi	0,0,r10,SPD_DIMM0
	bne	set_dimm1_size
	stw	r7,SD_D0_BAR(r4)
	sync
	li	r10,SPD_DIMM1
	READ_SPD(0)
	bne do_each_dimm
	b spd_done

set_dimm1_size:
	stw	r7,SD_D1_BAR(r4)
	sync
spd_done:
	blr

check_next_slot:
	cmpi	0,0,r10,SPD_DIMM1
	beq	spd_read_fail
	li	r10,SPD_DIMM1
	b	do_first_dimm
spd_read_fail:
	ori	r3,r0,0xdead
	b	err_hung
spd_fail:
	li	r3,0x0bad
	sync
err_hung:	/* hang here for debugging */
	nop
	nop
	b	err_hung

#endif /* !SDC_HARDCODED_INIT */
