/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl) and
 *		       Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 * 2004 (c) MontaVista Software, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include "config.h"
#include "version.h"


/*-----------------------------------------------------------------------
 * Board defines:
 */

#define MDCNFG		0x00
#define MDCAS00		0x04
#define MDCAS01		0x08
#define MDCAS02		0x0C
#define MSC0		0x10
#define MSC1		0x14
#define MECR		0x18
#define MDREFR		0x1C
#define MDCAS20		0x20
#define MDCAS21		0x24
#define MDCAS22		0x28
#define MSC2		0x2C
#define SMCNFG		0x30

#define ASSABET_BCR		(0x12000000)
#define ASSABET_BCR_DB1110	(0x00a07490 | (0<<16) | (0<<17))
#define ASSABET_SCR_nNEPONSET	(1 << 9)
#define NEPONSET_LEDS		(0x10000010)


/*-----------------------------------------------------------------------
 * Setup parameters for the board:
 */


MEM_BASE:	.long	0xa0000000
MEM_START:	.long	0xc0000000

mdcnfg:		.long	0x72547254
mdcas00:	.long	0xaaaaaa7f
mdcas01:	.long	0xaaaaaaaa
mdcas02:	.long	0xaaaaaaaa
msc0:		.long	0x4b384370
msc1:		.long	0x22212419
mecr:		.long	0x994a994a
mdrefr:		.long	0x04340327
mdcas20:	.long	0xaaaaaa7f
mdcas21:	.long	0xaaaaaaaa
mdcas22:	.long	0xaaaaaaaa
msc2:		.long	0x42196669
smcnfg:		.long	0x00000000

BCR:		.long	ASSABET_BCR
BCR_DB1110:	.long	ASSABET_BCR_DB1110
LEDS:		.long	NEPONSET_LEDS


	.globl lowlevel_init
lowlevel_init:

	/* Setting up the memory and stuff */

	ldr	r0, MEM_BASE
	ldr	r1, mdcas00
	str	r1, [r0, #MDCAS00]
	ldr	r1, mdcas01
	str	r1, [r0, #MDCAS01]
	ldr	r1, mdcas02
	str	r1, [r0, #MDCAS02]
	ldr	r1, mdcas20
	str	r1, [r0, #MDCAS20]
	ldr	r1, mdcas21
	str	r1, [r0, #MDCAS21]
	ldr	r1, mdcas22
	str	r1, [r0, #MDCAS22]
	ldr	r1, mdrefr
	str	r1, [r0, #MDREFR]
	ldr	r1, mecr
	str	r1, [r0, #MECR]
	ldr	r1, msc0
	str	r1, [r0, #MSC0]
	ldr	r1, msc1
	str	r1, [r0, #MSC1]
	ldr	r1, msc2
	str	r1, [r0, #MSC2]
	ldr	r1, smcnfg
	str	r1, [r0, #SMCNFG]

	ldr	r1, mdcnfg
	str	r1, [r0, #MDCNFG]

	/* Load something to activate bank */
	ldr	r2, MEM_START
.rept	8
	ldr	r3, [r2]
.endr

	/* Enable SDRAM */
	orr	r1, r1, #0x00000001
	str	r1, [r0, #MDCNFG]

	ldr	r1, BCR
	ldr	r2, BCR_DB1110
	str	r2, [r1]

	ldr	r1, LEDS
	mov	r0, #0x3
	str	r0, [r1]

	/* All done... */
	mov	pc, lr
