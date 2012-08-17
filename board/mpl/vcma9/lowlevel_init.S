/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl) and
 *                     Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 * Modified for MPL VCMA9 by
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 * (C) Copyright 2002, 2003, 2004, 2005
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


/* register definitions */

#define PLD_BASE	0x28000000
#define MISC_REG	0x103
#define SDRAM_REG	0x106
#define BWSCON		0x48000000
#define CLKBASE		0x4C000000
#define LOCKTIME	0x0
#define MPLLCON		0x4
#define UPLLCON		0x8
#define GPIOBASE	0x56000000
#define GSTATUS1	0xB0
#define FASTCPU		0x02

/* some parameters for the board */
/* BWSCON */
#define DW8			(0x0)
#define DW16			(0x1)
#define DW32			(0x2)
#define WAIT			(0x1<<2)
#define UBLB			(0x1<<3)

/* BANKSIZE */
#define BURST_EN		(0x1<<7)

/* BANK0CON 200 */
#define B0_Tacs_200		0x0	/*  0clk  (or 0x1 1clk) */
#define B0_Tcos_200		0x1	/*  1clk  (or 0x2 2clk) */
#define B0_Tacc_200		0x5	/*  8clk  (or 0x6 10clk) */
#define B0_Tcoh_200		0x0	/*  0clk */
#define B0_Tcah_200		0x3	/*  4clk  (or0x01 1clk) */
#define B0_Tacp_200		0x0     /* page mode is not used */
#define B0_PMC_200		0x0	/* page mode disabled */

/* BANK0CON 250 */
#define B0_Tacs_250		0x0	/*  0clk  (or 0x1 1clk) */
#define B0_Tcos_250		0x1	/*  1clk  (or 0x2 2clk) */
#define B0_Tacc_250		0x5	/*  8clk  (or 0x7 14clk) */
#define B0_Tcoh_250		0x0	/*  0clk */
#define B0_Tcah_250		0x3	/*  4clk  (or 0x1 1clk) */
#define B0_Tacp_250		0x0     /* page mode is not used */
#define B0_PMC_250		0x0	/* page mode disabled */

/* BANK0CON 266 */
#define B0_Tacs_266		0x0	/*  0clk  (or 0x1 1clk) */
#define B0_Tcos_266		0x1	/*  1clk  (or 0x2 2clk) */
#define B0_Tacc_266		0x6	/*  10clk (or 0x7 14clk) */
#define B0_Tcoh_266		0x0	/*  0clk */
#define B0_Tcah_266		0x3	/*  4clk  (or 0x1 1clk) */
#define B0_Tacp_266		0x0     /* page mode is not used */
#define B0_PMC_266		0x0	/* page mode disabled */

/* BANK1CON 200 */
#define B1_Tacs_200		0x0	/*  0clk  (or 0x1 1clk) */
#define B1_Tcos_200		0x1	/*  1clk  (or 0x2 2clk) */
#define B1_Tacc_200		0x5	/*  8clk  (or 0x6 10clk) */
#define B1_Tcoh_200		0x0	/*  0clk */
#define B1_Tcah_200		0x3	/*  4clk  (or 0x1 1clk) */
#define B1_Tacp_200		0x0     /* page mode is not used */
#define B1_PMC_200		0x0	/* page mode disabled */

/* BANK1CON 250 */
#define B1_Tacs_250		0x0	/*  0clk  (or 0x1 1clk) */
#define B1_Tcos_250		0x1	/*  1clk  (or 0x2 2clk) */
#define B1_Tacc_250		0x5	/*  8clk  (or 0x7 14clk) */
#define B1_Tcoh_250		0x0	/*  0clk */
#define B1_Tcah_250		0x3	/*  4clk  (or 0x1 1clk) */
#define B1_Tacp_250		0x0     /* page mode is not used */
#define B1_PMC_250		0x0	/* page mode disabled */

/* BANK1CON 266 */
#define B1_Tacs_266		0x0	/*  0clk  (or 0x1 1clk) */
#define B1_Tcos_266		0x1	/*  1clk  (or 0x2 2clk) */
#define B1_Tacc_266		0x6	/*  10clk (or 0x7 14clk) */
#define B1_Tcoh_266		0x0	/*  0clk */
#define B1_Tcah_266		0x3	/*  4clk  (or 0x1 1clk) */
#define B1_Tacp_266		0x0     /* page mode is not used */
#define B1_PMC_266		0x0	/* page mode disabled */

/* BANK2CON 200 + 250 + 266 */
#define B2_Tacs			0x3	/*  4clk */
#define B2_Tcos			0x3	/*  4clk */
#define B2_Tacc			0x7     /* 14clk */
#define B2_Tcoh			0x3	/*  4clk */
#define B2_Tcah			0x3	/*  4clk */
#define B2_Tacp			0x0	/* page mode is not used */
#define B2_PMC			0x0     /* page mode disabled */

/* BANK3CON 200 + 250 + 266 */
#define B3_Tacs			0x3	/*  4clk */
#define B3_Tcos			0x3	/*  4clk */
#define B3_Tacc			0x7     /* 14clk */
#define B3_Tcoh			0x3	/*  4clk */
#define B3_Tcah			0x3	/*  4clk */
#define B3_Tacp			0x0	/* page mode is not used */
#define B3_PMC			0x0     /* page mode disabled */

/* BANK4CON 200 */
#define B4_Tacs_200		0x1	/*  1clk */
#define B4_Tcos_200		0x3	/*  4clk */
#define B4_Tacc_200		0x7	/* 14clk */
#define B4_Tcoh_200		0x3	/*  4clk */
#define B4_Tcah_200		0x2	/*  2clk */
#define B4_Tacp_200		0x0     /* page mode is not used */
#define B4_PMC_200		0x0	/* page mode disabled */

/* BANK4CON 250 */
#define B4_Tacs_250		0x1	/*  1clk */
#define B4_Tcos_250		0x3	/*  4clk */
#define B4_Tacc_250		0x7	/* 14clk */
#define B4_Tcoh_250		0x3	/*  4clk */
#define B4_Tcah_250		0x2	/*  2clk */
#define B4_Tacp_250		0x0     /* page mode is not used */
#define B4_PMC_250		0x0	/* page mode disabled */

/* BANK4CON 266 */
#define B4_Tacs_266		0x1	/*  1clk */
#define B4_Tcos_266		0x3	/*  4clk */
#define B4_Tacc_266		0x7	/* 14clk */
#define B4_Tcoh_266		0x3	/*  4clk */
#define B4_Tcah_266		0x2	/*  2clk */
#define B4_Tacp_266		0x0     /* page mode is not used */
#define B4_PMC_266		0x0	/* page mode disabled */

/* BANK5CON 200 */
#define B5_Tacs_200		0x0	/*  0clk */
#define B5_Tcos_200		0x3	/*  4clk */
#define B5_Tacc_200		0x4	/*  6clk */
#define B5_Tcoh_200		0x3	/*  4clk */
#define B5_Tcah_200		0x1	/*  1clk */
#define B5_Tacp_200		0x0     /* page mode is not used */
#define B5_PMC_200		0x0	/* page mode disabled */

/* BANK5CON 250 */
#define B5_Tacs_250		0x0	/*  0clk */
#define B5_Tcos_250		0x3	/*  4clk */
#define B5_Tacc_250		0x5	/*  8clk */
#define B5_Tcoh_250		0x3	/*  4clk */
#define B5_Tcah_250		0x1	/*  1clk */
#define B5_Tacp_250		0x0     /* page mode is not used */
#define B5_PMC_250		0x0	/* page mode disabled */

/* BANK5CON 266 */
#define B5_Tacs_266		0x0	/*  0clk */
#define B5_Tcos_266		0x3	/*  4clk */
#define B5_Tacc_266		0x5	/*  8clk */
#define B5_Tcoh_266		0x3	/*  4clk */
#define B5_Tcah_266		0x1	/*  1clk */
#define B5_Tacp_266		0x0     /* page mode is not used */
#define B5_PMC_266		0x0	/* page mode disabled */

#define B6_MT			0x3	/* SDRAM */
#define B6_Trcd_200		0x0	/* 2clk */
#define B6_Trcd_250		0x1	/* 3clk */
#define B6_Trcd_266		0x1	/* 3clk */
#define B6_SCAN			0x2	/* 10bit */

#define B7_MT			0x3	/* SDRAM */
#define B7_Trcd_200		0x0	/* 2clk */
#define B7_Trcd_250		0x1	/* 3clk */
#define B7_Trcd_266		0x1	/* 3clk */
#define B7_SCAN			0x2	/* 10bit */

/* REFRESH parameter */
#define REFEN			0x1	/* Refresh enable */
#define TREFMD			0x0	/* CBR(CAS before RAS)/Auto refresh */
#define Trp_200			0x0	/* 2clk */
#define Trp_250			0x1	/* 3clk */
#define Trp_266			0x1	/* 3clk */
#define Tsrc_200		0x1	/* 5clk */
#define Tsrc_250		0x2	/* 6clk */
#define Tsrc_266		0x3	/* 7clk */

/* period=15.6us, HCLK=100Mhz, (2048+1-15.6*100) */
#define REFCNT_200		489
/* period=15.6us, HCLK=125Mhz, (2048+1-15.6*125) */
#define REFCNT_250		99
/* period=15.6us, HCLK=133Mhz, (2048+1-15.6*133) */
#define REFCNT_266		0
/**************************************/

_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE

.globl lowlevel_init
lowlevel_init:
	/* use r0 to relocate DATA read/write to flash rather than memory ! */
	ldr	r0, _TEXT_BASE
	ldr	r13, =BWSCON

	/* enable minimal access to PLD */
	ldr	r1, [r13]			/* load default BWSCON */
	orr	r1, r1, #(DW8 + UBLB) << 20	/* set necessary CS attrs */
	str	r1, [r13]			/* set BWSCON */
	ldr	r1, =0x7FF0			/* select slowest timing */
	str	r1, [r13, #0x18]		/* set BANKCON5 */

	ldr	r1, =PLD_BASE
	ldr	r2, =SETUPDATA
	ldrb	r1, [r1, #MISC_REG]
	sub	r2, r2, r0
	tst	r1, #FASTCPU			/* FASTCPU available ? */
	addeq	r2, r2, #SETUPENTRY_SIZE

	/* memory control configuration */
	/* r2 = pointer into timing table */
	/* r13 = pointer to MEM controller regs (starting with BWSCON) */
	add     r3, r2, #CSDATA_OFFSET
	add     r4, r3, #CSDATAENTRY_SIZE
0:
	ldr     r1, [r3], #4
	str     r1, [r13], #4
	cmp     r3, r4
	bne     0b

	/* PLD access is now possible */
	/* r3 = SDRAMDATA
	/* r13 = pointer to MEM controller regs */
	ldr	r1, =PLD_BASE
	mov	r4, #SDRAMENTRY_SIZE
	ldrb	r1, [r1, #SDRAM_REG]
	/* calculate start and end point */
	mla	r3, r4, r1, r3
	add     r4, r3, r4
0:
	ldr     r1, [r3], #4
	str     r1, [r13], #4
	cmp     r3, r4
	bne     0b

	/* setup MPLL registers */
	ldr	r1, =CLKBASE
	ldr	r4, =0xFFFFFF
	add	r3, r2, #4		/* r3 points to PLL values */
	str	r4, [r1, #LOCKTIME]
	ldmia	r3, {r4,r5}
	str	r5, [r1, #UPLLCON]	/* writing PLL register */
					/* !! order seems to be important !! */
	/* a little delay */
	ldr	r3, =0x4000
0:
	subs	r3, r3, #1
	bne	0b

	str	r4, [r1, #MPLLCON]	/* writing PLL register */
					/* !! order seems to be important !! */
	/* a little delay */
	ldr	r3, =0x4000
0:
	subs	r3, r3, #1
	bne	0b

	/* everything is fine now */
	mov	pc, lr

	.ltorg
/* the literal pools origin */

#define MK_BWSCON(bws1, bws2, bws3, bws4, bws5, bws6, bws7) \
	((bws1) << 4) + \
	((bws2) << 8) + \
	((bws3) << 12) + \
	((bws4) << 16) + \
	((bws5) << 20) + \
	((bws6) << 24) + \
	((bws7) << 28)

#define MK_BANKCON(tacs, tcos, tacc, tcoh, tcah, tacp, pmc) \
	((tacs) << 13) + \
	((tcos) << 11) + \
	((tacc) << 8) + \
	((tcoh) << 6) + \
	((tcah) << 4) + \
	((tacp) << 2) + \
	(pmc)

#define MK_BANKCON_SDRAM(trcd, scan) \
	((0x03) << 15) + \
	((trcd) << 2) + \
	(scan)

#define MK_SDRAM_REFRESH(enable, trefmd, trp, tsrc, cnt) \
	((enable) << 23) + \
	((trefmd) << 22) + \
	((trp) << 20) + \
	((tsrc) << 18) + \
	(cnt)

SETUPDATA:
	.word 0x32410002
	/* PLL values (MDIV, PDIV, SDIV) for 250 MHz */
	.word (0x75 << 12) + (0x01 << 4) + (0x01 << 0)
	/* PLL values for USB clock */
	.word (0x48 << 12) + (0x03 << 4) + (0x02 << 0)

	/* timing for 250 MHz*/
0:
	.equiv CSDATA_OFFSET, (. - SETUPDATA)
	.word MK_BWSCON(DW16, \
			DW32, \
			DW32, \
			DW16 + WAIT + UBLB, \
			DW8 + UBLB, \
			DW32, \
			DW32)

	.word MK_BANKCON(B0_Tacs_250, \
			 B0_Tcos_250, \
			 B0_Tacc_250, \
			 B0_Tcoh_250, \
			 B0_Tcah_250, \
			 B0_Tacp_250, \
			 B0_PMC_250)

	.word MK_BANKCON(B1_Tacs_250, \
			 B1_Tcos_250, \
			 B1_Tacc_250, \
			 B1_Tcoh_250, \
			 B1_Tcah_250, \
			 B1_Tacp_250, \
			 B1_PMC_250)

	.word MK_BANKCON(B2_Tacs, \
			 B2_Tcos, \
			 B2_Tacc, \
			 B2_Tcoh, \
			 B2_Tcah, \
			 B2_Tacp, \
			 B2_PMC)

	.word MK_BANKCON(B3_Tacs, \
			 B3_Tcos, \
			 B3_Tacc, \
			 B3_Tcoh, \
			 B3_Tcah, \
			 B3_Tacp, \
			 B3_PMC)

	.word MK_BANKCON(B4_Tacs_250, \
			 B4_Tcos_250, \
			 B4_Tacc_250, \
			 B4_Tcoh_250, \
			 B4_Tcah_250, \
			 B4_Tacp_250, \
			 B4_PMC_250)

	.word MK_BANKCON(B5_Tacs_250, \
			 B5_Tcos_250, \
			 B5_Tacc_250, \
			 B5_Tcoh_250, \
			 B5_Tcah_250, \
			 B5_Tacp_250, \
			 B5_PMC_250)

	.equiv CSDATAENTRY_SIZE, (. - 0b)
	/* 4Mx8x4 */
0:
	.word MK_BANKCON_SDRAM(B6_Trcd_250, B6_SCAN)
	.word MK_BANKCON_SDRAM(B7_Trcd_250, B7_SCAN)
	.word MK_SDRAM_REFRESH(REFEN, TREFMD, Trp_250, Tsrc_250, REFCNT_250)
	.word 0x32 + BURST_EN
	.word 0x30
	.word 0x30
	.equiv SDRAMENTRY_SIZE, (. - 0b)

	/* 8Mx8x4 */
	.word MK_BANKCON_SDRAM(B6_Trcd_250, B6_SCAN)
	.word MK_BANKCON_SDRAM(B7_Trcd_250, B7_SCAN)
	.word MK_SDRAM_REFRESH(REFEN, TREFMD, Trp_250, Tsrc_250, REFCNT_250)
	.word 0x32 + BURST_EN
	.word 0x30
	.word 0x30

	/* 2Mx8x4 */
	.word MK_BANKCON_SDRAM(B6_Trcd_250, B6_SCAN)
	.word MK_BANKCON_SDRAM(B7_Trcd_250, B7_SCAN)
	.word MK_SDRAM_REFRESH(REFEN, TREFMD, Trp_250, Tsrc_250, REFCNT_250)
	.word 0x32 + BURST_EN
	.word 0x30
	.word 0x30

	/* 4Mx8x2 */
	.word MK_BANKCON_SDRAM(B6_Trcd_250, B6_SCAN)
	.word MK_BANKCON_SDRAM(B7_Trcd_250, B7_SCAN)
	.word MK_SDRAM_REFRESH(REFEN, TREFMD, Trp_250, Tsrc_250, REFCNT_250)
	.word 0x32 + BURST_EN
	.word 0x30
	.word 0x30

	.equiv SETUPENTRY_SIZE, (. - SETUPDATA)

	.word 0x32410000
	/* PLL values (MDIV, PDIV, SDIV) for 200 MHz (Fout = 202.8MHz) */
	.word (0xA1 << 12) + (0x03 << 4) + (0x01 << 0)
	/* PLL values for USB clock */
	.word (0x48 << 12) + (0x03 << 4) + (0x02 << 0)

	/* timing for 200 MHz and default*/
	.word MK_BWSCON(DW16, \
			DW32, \
			DW32, \
			DW16 + WAIT + UBLB, \
			DW8 + UBLB, \
			DW32, \
			DW32)

	.word MK_BANKCON(B0_Tacs_200, \
			 B0_Tcos_200, \
			 B0_Tacc_200, \
			 B0_Tcoh_200, \
			 B0_Tcah_200, \
			 B0_Tacp_200, \
			 B0_PMC_200)

	.word MK_BANKCON(B1_Tacs_200, \
			 B1_Tcos_200, \
			 B1_Tacc_200, \
			 B1_Tcoh_200, \
			 B1_Tcah_200, \
			 B1_Tacp_200, \
			 B1_PMC_200)

	.word MK_BANKCON(B2_Tacs, \
			 B2_Tcos, \
			 B2_Tacc, \
			 B2_Tcoh, \
			 B2_Tcah, \
			 B2_Tacp, \
			 B2_PMC)

	.word MK_BANKCON(B3_Tacs, \
			 B3_Tcos, \
			 B3_Tacc, \
			 B3_Tcoh, \
			 B3_Tcah, \
			 B3_Tacp, \
			 B3_PMC)

	.word MK_BANKCON(B4_Tacs_200, \
			 B4_Tcos_200, \
			 B4_Tacc_200, \
			 B4_Tcoh_200, \
			 B4_Tcah_200, \
			 B4_Tacp_200, \
			 B4_PMC_200)

	.word MK_BANKCON(B5_Tacs_200, \
			 B5_Tcos_200, \
			 B5_Tacc_200, \
			 B5_Tcoh_200, \
			 B5_Tcah_200, \
			 B5_Tacp_200, \
			 B5_PMC_200)

	/* 4Mx8x4 */
	.word MK_BANKCON_SDRAM(B6_Trcd_200, B6_SCAN)
	.word MK_BANKCON_SDRAM(B7_Trcd_200, B7_SCAN)
	.word MK_SDRAM_REFRESH(REFEN, TREFMD, Trp_200, Tsrc_200, REFCNT_200)
	.word 0x32 + BURST_EN
	.word 0x30
	.word 0x30

	/* 8Mx8x4 */
	.word MK_BANKCON_SDRAM(B6_Trcd_200, B6_SCAN)
	.word MK_BANKCON_SDRAM(B7_Trcd_200, B7_SCAN)
	.word MK_SDRAM_REFRESH(REFEN, TREFMD, Trp_200, Tsrc_200, REFCNT_200)
	.word 0x32 + BURST_EN
	.word 0x30
	.word 0x30

	/* 2Mx8x4 */
	.word MK_BANKCON_SDRAM(B6_Trcd_200, B6_SCAN)
	.word MK_BANKCON_SDRAM(B7_Trcd_200, B7_SCAN)
	.word MK_SDRAM_REFRESH(REFEN, TREFMD, Trp_200, Tsrc_200, REFCNT_200)
	.word 0x32 + BURST_EN
	.word 0x30
	.word 0x30

	/* 4Mx8x2 */
	.word MK_BANKCON_SDRAM(B6_Trcd_200, B6_SCAN)
	.word MK_BANKCON_SDRAM(B7_Trcd_200, B7_SCAN)
	.word MK_SDRAM_REFRESH(REFEN, TREFMD, Trp_200, Tsrc_200, REFCNT_200)
	.word 0x32 + BURST_EN
	.word 0x30
	.word 0x30

	.equiv SETUPDATA_SIZE, (. - SETUPDATA)
