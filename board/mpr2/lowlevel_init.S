/*
 * (C) Copyright 2008
 * Mark Jonas <mark.jonas@de.bosch.com>
 *
 * (C) Copyright 2007
 * Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
 *
 * board/mpr2/lowlevel_init.S
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <asm/macro.h>

	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:

/*
 * Set frequency multipliers and dividers in FRQCR.
 */
	write16	WTCSR_A, WTCSR_D

	write16	WTCNT_A, WTCNT_D

	write16	FRQCR_A, FRQCR_D

/*
 * Setup CS0 (Flash).
 */
	write32	CS0BCR_A, CS0BCR_D

	write32	CS0WCR_A, CS0WCR_D

/*
 * Setup CS3 (SDRAM).
 */
	write32	CS3BCR_A, CS3BCR_D

	write32	CS3WCR_A, CS3WCR_D

	write32	SDCR_A, SDCR_D1

	write32	RTCSR_A, RTCSR_D

	write32	RTCNT_A, RTCNT_D

	write32	RTCOR_A, RTCOR_D

	write32	SDCR_A, SDCR_D2

	mov.l	SDMR3_A, r1
	mov.l	SDMR3_D, r0
	add	r0, r1
	mov	#0, r0
	mov.w	r0, @r1

	rts
	nop

	.align 4

/*
 * Configuration for MPR2 A.3 through A.7
 */

/*
 * PLL Settings
 */
FRQCR_D:	.word	0x1103		/* I:B:P=8:4:2 */
WTCNT_D:	.word	0x5A00		/* start counting at zero */
WTCSR_D:	.word	0xA507		/* divide by 4096 */
.align 2
/*
 * Spansion S29GL256N11 @ 48 MHz
 */
/* 1 idle cycle inserted, normal space, 16 bit */
CS0BCR_D:	.long	0x12490400
/* tSW=0.5ck, 6 wait cycles, NO external wait, tHW=0.5ck */
CS0WCR_D:	.long	0x00000340

/*
 * Samsung K4S511632B-UL75 @ 48 MHz
 * Micron MT48LC32M16A2-75 @ 48 MHz
 */
/* CS3BCR = 0x10004400, minimum idle cycles, SDRAM, 16 bit */
CS3BCR_D:	.long	0x10004400
/* tRP=1ck, tRCD=1ck, CL=2, tRWL=2ck, tRC=4ck */
CS3WCR_D:	.long	0x00000091
/* no refresh, 13 rows, 10 cols, NO bank active mode */
SDCR_D1:	.long	0x00000012
SDCR_D2:	.long	0x00000812	/* refresh */
RTCSR_D:	.long	0xA55A0008	/* 1/4, once */
RTCNT_D:	.long	0xA55A005D	/* count 93 */
RTCOR_D:	.long	0xa55a005d	/* count 93 */
/* mode register CL2, burst read and SINGLE WRITE */
SDMR3_D:	.long	0x440

/*
 * Registers
 */

FRQCR_A:	.long	0xA415FF80
WTCNT_A:	.long	0xA415FF84
WTCSR_A:	.long	0xA415FF86

#define BSC_BASE	0xA4FD0000
CS0BCR_A:	.long	BSC_BASE + 0x04
CS3BCR_A:	.long	BSC_BASE + 0x0C
CS0WCR_A:	.long	BSC_BASE + 0x24
CS3WCR_A:	.long	BSC_BASE + 0x2C
SDCR_A:		.long	BSC_BASE + 0x44
RTCSR_A:	.long	BSC_BASE + 0x48
RTCNT_A:	.long	BSC_BASE + 0x4C
RTCOR_A:	.long	BSC_BASE + 0x50
SDMR3_A:	.long	BSC_BASE + 0x5000
