/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl) and
 *                     Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
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


#include "config.h"
#include "version.h"


/* some parameters for the board */

MEM_BASE:	.long	0xa0000000
MEM_START:	.long	0xc0000000

#define MDCNFG		0x00
#define MDCAS00		0x04		/* CAS waveform rotate reg 0       */
#define MDCAS01		0x08		/* CAS waveform rotate reg 1 bank  */
#define MDCAS02		0x0C		/* CAS waveform rotate reg 2 bank  */
#define MDREFR		0x1C		/* DRAM refresh control reg        */
#define MDCAS20		0x20		/* CAS waveform rotate reg 0 bank  */
#define MDCAS21		0x24		/* CAS waveform rotate reg 1 bank  */
#define MDCAS22		0x28		/* CAS waveform rotate reg 2 bank  */
#define MECR		0x18		/* Expansion memory (PCMCIA) bus configuration  register */
#define MSC0		0x10		/* static memory control reg 0     */
#define MSC1		0x14		/* static memory control reg 1     */
#define MSC2		0x2C		/* static memory control reg 2     */
#define SMCNFG		0x30		/* SMROM configuration reg         */

mdcas00:	.long	0x5555557F
mdcas01:	.long	0x55555555
mdcas02:	.long	0x55555555
mdcas20:	.long	0x5555557F
mdcas21:	.long	0x55555555
mdcas22:	.long	0x55555555
mdcnfg:		.long	0x0000B25C
mdrefr:		.long	0x007000C1
mecr:		.long	0x10841084
msc0:		.long	0x00004774
msc1:		.long	0x00000000
msc2:		.long	0x00000000
smcnfg:		.long	0x00000000

/* setting up the memory */

.globl lowlevel_init
lowlevel_init:

	ldr	r0, MEM_BASE

	/* Set up the DRAM */

	/* MDCAS00 */
	ldr	r1, mdcas00
	str	r1, [r0, #MDCAS00]

	/* MDCAS01 */
	ldr	r1, mdcas01
	str	r1, [r0, #MDCAS01]

	/* MDCAS02 */
	ldr	r1, mdcas02
	str	r1, [r0, #MDCAS02]

	/* MDCAS20 */
	ldr	r1, mdcas20
	str	r1, [r0, #MDCAS20]

	/* MDCAS21 */
	ldr	r1, mdcas21
	str	r1, [r0, #MDCAS21]

	/* MDCAS22 */
	ldr	r1, mdcas22
	str	r1, [r0, #MDCAS22]

	/* MDREFR */
	ldr	r1, mdrefr
	str	r1, [r0, #MDREFR]

	/* Set up PCMCIA space */
	ldr	r1, mecr
	str	r1, [r0, #MECR]

	/* Setup the flash memory and other */
	ldr	r1, msc0
	str	r1, [r0, #MSC0]

	ldr	r1, msc1
	str	r1, [r0, #MSC1]

	ldr	r1, msc2
	str	r1, [r0, #MSC2]

	ldr	r1, smcnfg
	str	r1, [r0, #SMCNFG]

	/* MDCNFG */
	ldr	r1, mdcnfg
	bic	r1, r1, #0x00000001
	str	r1, [r0, #MDCNFG]

	/* Load something to activate bank */
	ldr	r2, MEM_START
.rept	8
	ldr	r1, [r2]
.endr

	/* MDCNFG */
	ldr	r1, mdcnfg
	orr	r1, r1, #0x00000001
	str	r1, [r0, #MDCNFG]

	/* everything is fine now */
	mov	pc, lr
