/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl) and
 *		       Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 * 2004 (c) MontaVista Software, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
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

#define GPDR	0x04
#define GPSR	0x08
#define GPCR	0x0C
#define GAFR	0x1C

#define PPDR	0x00
#define PPSR	0x04
#define PPAR	0x08

#define MDREFR_TRASR(n_) (n_ & (0x0000000f))
#define MDREFR_DRI(n_)   ((n_ & (0x00000fff)) << 4)
#define MDREFR_K0DB2 (1 << 18)
#define MDREFR_K1DB2 (1 << 22)
#define MDREFR_K2DB2 (1 << 26)

#define MDREFR_K0RUN (1 << 17)
#define MDREFR_K1RUN (1 << 21)
#define MDREFR_K2RUN (1 << 25)

#define MDREFR_SLFRSH (1 << 31)
#define MDREFR_E1PIN  (1 << 20)

#define PSSR    0x04
#define PSSR_DH 0x00000008
#define POSR    0x08
#define RCSR    0x04

/*-----------------------------------------------------------------------
 * Setup parameters for the board:
 */
MEM_BASE:	.long	0xa0000000
MEM_START:	.long	0xc0000000
PWR_BASE:	.word	0x90020000
RST_BASE:	.long	0x90030000
PPC_BASE:	.long	0x90060000
GPIO_BASE:	.long	0x90040000
IC_BASE:	.word	0x90050000

cpuspeed:	.word	0xa0
/* calculated from old blob bootloader */
mdcnfg:	.long	0x00037267	/* mdcnfg  0x00037267 */
mdcas00:	.long	0x5555557f	/* mdcas00 0x5555557f */
mdcas01:	.long	0x55555555	/* mdcas01 0x55555555 */
mdcas02:	.long	0x55555555	/* mdcas02 0x55555555 */
msc0:	.long	0xfff04f78		/* msc0    0xfff04f78 */
msc1:	.long	0xfff8fff0		/* msc1    0xfff8fff0 */
mecr:	.long	0x98c698c6	/* mecr    0x98c698c6 */
mdrefr:	.long	0x067600c7	/* mdrefr  0x04340327 */
mdcas20:	.long	0xd1284142	/* mdcas20 0xd1284142 */
mdcas21:	.long	0x72249529	/* mdcas21 0x72249529 */
mdcas22:	.long	0x78414351	/* mdcas22 0x78414351 */
msc2:	.long	0x201d2959		/* msc2    0x201d2959 */
smcnfg:	.long	0x00000000	/* smcnfg  0x00000000 */

pin_set_out:	.long	0x37ff70
pin_set_dir:	.long	0x11480

gpdr_set:	.long	0x0B3A0900
gpsr_set:	.long	0x02100800
gpcr_set:	.long	0x092A0100
gafr_set:	.long	0x08600000

.globl lowlevel_init
lowlevel_init:


	/* this is required for flashing */
	ldr	r0, PPC_BASE
	ldr	r1, pin_set_out
	str	r1, [r0, #PPSR]
	ldr	r1, pin_set_dir
	str	r1, [r0, #PPDR]

	/* Setting up the memory and stuff */
	/***********************************/

	ldr	r0, MEM_BASE

	ldr	r1, mdcnfg
	str	r1, [r0, #MDCNFG]
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

	/* clear kxDB2 */
	ldr	r2, [r0, #MDREFR]
	bic	r2, r2, #MDREFR_K0DB2
	bic	r2, r2, #MDREFR_K1DB2
	bic	r2, r2, #MDREFR_K2DB2
	str	r2, [r0, #MDREFR]

	ldr	r2, [r0, #MDREFR]
	orr r2, r2, #MDREFR_TRASR(7)

	mov r4, #0x2000
	spin:	subs	r4, r4, #1
	bne	spin

	ldr	r1, PWR_BASE
	mov	r2, #PSSR_DH
	str	r2, [r1, #PSSR]

	ldr	r2, [r0, #MDREFR]
	bic	r2, r2, #MDREFR_K0DB2
	bic	r2, r2, #MDREFR_K1DB2
	bic	r2, r2, #MDREFR_K2DB2
	str	r2, [r0, #MDREFR]

	ldr	r2, [r0, #MDREFR]
	orr	r2, r2, #MDREFR_TRASR(7)
	orr	r2, r2, #MDREFR_DRI(12)
	orr	r2, r2, #MDREFR_K0DB2
	orr	r2, r2, #MDREFR_K1DB2
	orr	r2, r2, #MDREFR_K2DB2
	str	r2, [r0, #MDREFR]

	ldr	r2, [r0, #MDREFR]
	orr	r2, r2, #MDREFR_K0RUN
	orr	r2, r2, #MDREFR_K1RUN
	orr	r2, r2, #MDREFR_K2RUN
	str	r2, [r0, #MDREFR]

	ldr	r2, [r0, #MDREFR]
	bic	r2, r2, #MDREFR_SLFRSH
	str	r2, [r0, #MDREFR]

	ldr	r2, [r0, #MDREFR]
	orr	r2, r2, #MDREFR_E1PIN
	str	r2, [r0, #MDREFR]

	ldr	r2, MEM_START
.rept	8
	ldr	r3, [r2]
.endr

	ldr	r2, [r0, #MDCNFG]
	orr	r2, r2, #0x00000003
	orr	r2, r2, #0x00030000
	str	r2, [r0, #MDCNFG]

	ldr	r1, msc0
	str	r1, [r0, #MSC0]
	ldr	r1, msc1
	str	r1, [r0, #MSC1]
	ldr	r1, msc2
	str	r1, [r0, #MSC2]
	ldr	r1, smcnfg
	str	r1, [r0, #SMCNFG]
	ldr	r1, mecr
	str	r1, [r0, #MECR]

	mov	pc, lr
