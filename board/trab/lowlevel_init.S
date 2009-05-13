/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl) and
 *                     Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 * Modified for the TRAB board by
 * (C) Copyright 2002-2003
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
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


/* some parameters for the board */

/*
 *
 * Copied from linux/arch/arm/boot/compressed/head-s3c2400.S
 *
 * Copyright (C) 2001 Samsung Electronics by chc, 010406
 *
 * TRAB specific tweaks.
 *
 */

/* memory controller */
#define BWSCON 0x14000000

/* Bank0 */
#define B0_Tacs	0x1	/* 1 clk */
#define B0_Tcos	0x1	/* 1 clk */
#define B0_Tacc	0x5	/* 8 clk */
#define B0_Tcoh	0x1	/* 1 clk */
#define B0_Tah	0x1	/* 1 clk */
#define B0_Tacp	0x0
#define B0_PMC	0x0	/* normal */

/* Bank1 - SRAM */
#define B1_Tacs	0x1	/* 1 clk */
#define B1_Tcos	0x1	/* 1 clk */
#define B1_Tacc	0x5	/* 8 clk */
#define B1_Tcoh	0x1	/* 1 clk */
#define B1_Tah	0x1	/* 1 clk */
#define B1_Tacp	0x0
#define B1_PMC	0x0	/* normal */

/* Bank2 - CPLD */
#define B2_Tacs	0x1	/* 1 clk */
#define B2_Tcos	0x1	/* 1 clk */
#define B2_Tacc	0x5	/* 8 clk */
#define B2_Tcoh	0x1	/* 1 clk */
#define B2_Tah	0x1	/* 1 clk */
#define B2_Tacp	0x0
#define B2_PMC	0x0	/* normal */

/* Bank3 - setup for the cs8900 */
#define B3_Tacs	0x3	/* 4 clk */
#define B3_Tcos	0x3	/* 4 clk */
#define B3_Tacc	0x7	/* 14 clk */
#define B3_Tcoh	0x1	/* 1 clk */
#define B3_Tah	0x0	/* 0 clk */
#define B3_Tacp	0x3	/* 6 clk */
#define B3_PMC	0x0	/* normal */

/* Bank4 */
#define B4_Tacs	0x0	/* 0 clk */
#define B4_Tcos	0x0	/* 0 clk */
#define B4_Tacc	0x7	/* 14 clk */
#define B4_Tcoh	0x0	/* 0 clk */
#define B4_Tah	0x0	/* 0 clk */
#define B4_Tacp	0x0
#define B4_PMC	0x0	/* normal */

/* Bank5 */
#define B5_Tacs	0x0	/* 0 clk */
#define B5_Tcos	0x0	/* 0 clk */
#define B5_Tacc	0x7	/* 14 clk */
#define B5_Tcoh	0x0	/* 0 clk */
#define B5_Tah	0x0	/* 0 clk */
#define B5_Tacp	0x0
#define B5_PMC	0x0	/* normal */

#ifndef CONFIG_RAM_16MB		/* 32 MB RAM */
/* Bank6 */
#define	B6_MT	0x3	/* SDRAM */
#define	B6_Trcd	0x0	/* 2clk */
#define	B6_SCAN	0x1	/* 9 bit */

/* Bank7 */
#define	B7_MT	0x3	/* SDRAM */
#define	B7_Trcd	0x0	/* 2clk */
#define	B7_SCAN	0x1	/* 9 bit */
#else	/* CONFIG_RAM_16MB	=  16 MB RAM */
/* Bank6 */
#define	B6_MT	0x3	/* SDRAM */
#define	B6_Trcd	0x1	/* 2clk */
#define	B6_SCAN	0x0	/* 8 bit */

/* Bank7 */
#define	B7_MT	0x3	/* SDRAM */
#define	B7_Trcd	0x1	/* 2clk */
#define	B7_SCAN	0x0	/* 8 bit */
#endif	/* CONFIG_RAM_16MB */

/* refresh parameter */
#define REFEN	0x1	/* enable refresh */
#define TREFMD	0x0	/* CBR(CAS before RAS)/auto refresh */
#define Trp	0x0	/* 2 clk */
#define Trc	0x3	/* 7 clk */
#define Tchr	0x2	/* 3 clk */

#ifdef CONFIG_TRAB_50MHZ
#define REFCNT	1269	/* period=15.6 us, HCLK=50Mhz, (2048+1-15.6*50) */
#else
#define REFCNT	1011	/* period=15.6 us, HCLK=66.5Mhz, (2048+1-15.6*66.5) */
#endif


_TEXT_BASE:
	.word	TEXT_BASE

.globl lowlevel_init
lowlevel_init:
	/* memory control configuration */
	/* make r0 relative the current location so that it */
	/* reads SMRDATA out of FLASH rather than memory ! */
	ldr     r0, =SMRDATA
	ldr	r1, _TEXT_BASE
	sub	r0, r0, r1
	ldr	r1, =BWSCON	/* Bus Width Status Controller */
	add     r2, r0, #52
0:
	ldr     r3, [r0], #4
	str     r3, [r1], #4
	cmp     r2, r0
	bne     0b

	/* everything is fine now */
	mov	pc, lr

	.ltorg
/* the literal pools origin */

SMRDATA:
	.word	0x2211d644	/* d->Ethernet, 6->CPLD, 4->SRAM, 4->FLASH */
	.word	((B0_Tacs<<13)+(B0_Tcos<<11)+(B0_Tacc<<8)+(B0_Tcoh<<6)+(B0_Tah<<4)+(B0_Tacp<<2)+(B0_PMC)) /* GCS0 */
	.word	((B1_Tacs<<13)+(B1_Tcos<<11)+(B1_Tacc<<8)+(B1_Tcoh<<6)+(B1_Tah<<4)+(B1_Tacp<<2)+(B1_PMC)) /* GCS1 */
	.word	((B2_Tacs<<13)+(B2_Tcos<<11)+(B2_Tacc<<8)+(B2_Tcoh<<6)+(B2_Tah<<4)+(B2_Tacp<<2)+(B2_PMC)) /* GCS2 */
	.word	((B3_Tacs<<13)+(B3_Tcos<<11)+(B3_Tacc<<8)+(B3_Tcoh<<6)+(B3_Tah<<4)+(B3_Tacp<<2)+(B3_PMC)) /* GCS3 */
	.word	((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC)) /* GCS4 */
	.word	((B5_Tacs<<13)+(B5_Tcos<<11)+(B5_Tacc<<8)+(B5_Tcoh<<6)+(B5_Tah<<4)+(B5_Tacp<<2)+(B5_PMC)) /* GCS5 */
	.word	((B6_MT<<15)+(B6_Trcd<<2)+(B6_SCAN)) /* GCS6 */
	.word	((B7_MT<<15)+(B7_Trcd<<2)+(B7_SCAN)) /* GCS7 */
	.word	((REFEN<<23)+(TREFMD<<22)+(Trp<<20)+(Trc<<18)+(Tchr<<16)+REFCNT)
#ifndef CONFIG_RAM_16MB		/* 32 MB RAM */
	.word	0x10	/* BUSWIDTH=32, SCLK power saving mode, BANKSIZE 32M/32M */
#else	/* CONFIG_RAM_16MB	=  16 MB RAM */
	.word	0x17	/* BUSWIDTH=32, SCLK power saving mode, BANKSIZE 16M/16M */
#endif	/* CONFIG_RAM_16MB */
	.word	0x20	/* MRSR6, CL=2clk */
	.word	0x20	/* MRSR7 */
