/*
 * (C) Copyright 2007
 * Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
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

	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:

	mov.l	WTCSR_A,r1
	mov.l	WTCSR_D,r0
	mov.w	r0,@r1

	mov.l	WTCNT_A,r1
	mov.l	WTCNT_D,r0
	mov.w	r0,@r1

	mov.l	FRQCR_A,r1
	mov.l	FRQCR_D,r0
	mov.w	r0,@r1

	mov.l	UCLKCR_A,r1
	mov.l	UCLKCR_D,r0
	mov.w	r0,@r1

	mov.l	CMNCR_A, r1
	mov.l	CMNCR_D, r0
	mov.l	r0, @r1

	mov.l	CS0BCR_A, r1
	mov.l	CS0BCR_D, r0
	mov.l	r0, @r1

	mov.l	CS2BCR_A, r1
	mov.l	CS2BCR_D, r0
	mov.l	r0, @r1

	mov.l	CS3BCR_A, r1
	mov.l	CS3BCR_D, r0
	mov.l	r0, @r1

	mov.l	CS4BCR_A, r1
	mov.l	CS4BCR_D, r0
	mov.l	r0, @r1

	mov.l	CS5ABCR_A, r1
	mov.l	CS5ABCR_D, r0
	mov.l	r0, @r1

	mov.l	CS5BBCR_A, r1
	mov.l	CS5BBCR_D, r0
	mov.l	r0, @r1

	mov.l	CS6ABCR_A, r1
	mov.l	CS6ABCR_D, r0
	mov.l	r0, @r1

	mov.l	CS6BBCR_A, r1
	mov.l	CS6BBCR_D, r0
	mov.l	r0, @r1

	mov.l	CS0WCR_A, r1
	mov.l	CS0WCR_D, r0
	mov.l	r0, @r1

	mov.l	CS2WCR_A, r1
	mov.l	CS2WCR_D, r0
	mov.l	r0, @r1

	mov.l	CS3WCR_A, r1
	mov.l	CS3WCR_D, r0
	mov.l	r0, @r1

	mov.l	CS4WCR_A, r1
	mov.l	CS4WCR_D, r0
	mov.l	r0, @r1

	mov.l	CS5AWCR_A, r1
	mov.l	CS5AWCR_D, r0
	mov.l	r0, @r1

	mov.l	CS5BWCR_A, r1
	mov.l	CS5BWCR_D, r0
	mov.l	r0, @r1

	mov.l	CS6AWCR_A, r1
	mov.l	CS6AWCR_D, r0
	mov.l	r0, @r1

	mov.l	CS6BWCR_A, r1
	mov.l	CS6BWCR_D, r0
	mov.l	r0, @r1

	mov.l	SDCR_A, r1
	mov.l	SDCR_D1, r0
	mov.l	r0, @r1

	mov.l	RTCSR_A, r1
	mov.l	RTCSR_D, r0
	mov.l	r0, @r1

	mov.l	RTCNT_A, r1
	mov.l	RTCNT_D, r0
	mov.l	r0, @r1

	mov.l	RTCOR_A, r1
	mov.l	RTCOR_D, r0
	mov.l	r0, @r1

	mov.l	SDCR_A, r1
	mov.l	SDCR_D2, r0
	mov.l	r0, @r1

	mov.l	SDMR3_A, r1
	mov.l	SDMR3_D, r0
	mov.w	r0, @r1

	mov.l	PCCR_A, r1
	mov.l	PCCR_D, r0
	mov.w	r0, @r1

	mov.l	PDCR_A, r1
	mov.l	PDCR_D, r0
	mov.w	r0, @r1

	mov.l	PECR_A, r1
	mov.l	PECR_D, r0
	mov.w	r0, @r1

	mov.l	PGCR_A, r1
	mov.l	PGCR_D, r0
	mov.w	r0, @r1

	mov.l	PHCR_A, r1
	mov.l	PHCR_D, r0
	mov.w	r0, @r1

	mov.l	PPCR_A, r1
	mov.l	PPCR_D, r0
	mov.w	r0, @r1

	mov.l	PTCR_A, r1
	mov.l	PTCR_D, r0
	mov.w	r0, @r1

	mov.l	PVCR_A, r1
	mov.l	PVCR_D, r0
	mov.w	r0, @r1

	mov.l	PSELA_A, r1
	mov.l	PSELA_D, r0
	mov.w	r0, @r1

	mov.l	CCR_A, r1
	mov.l	CCR_D, r0
	mov.l	r0, @r1

	mov.l	LED_A, r1
	mov.l	LED_D, r0
	mov.b	r0, @r1

	rts
	 nop

	.align 4

FRQCR_A:	.long	0xA415FF80	/* FRQCR Address */
WTCNT_A:	.long	0xA415FF84
WTCSR_A:	.long	0xA415FF86
UCLKCR_A:	.long	0xA40A0008
FRQCR_D:	.long	0x1103		/* I:B:P=8:4:2 */
WTCNT_D:	.long	0x5A00
WTCSR_D:	.long	0xA506
UCLKCR_D:	.long	0xA5C0

#define BSC_BASE	0xA4FD0000
CMNCR_A:	.long	BSC_BASE
CS0BCR_A:	.long	BSC_BASE + 0x04
CS2BCR_A:	.long	BSC_BASE + 0x08
CS3BCR_A:	.long	BSC_BASE + 0x0C
CS4BCR_A:	.long	BSC_BASE + 0x10
CS5ABCR_A:	.long	BSC_BASE + 0x14
CS5BBCR_A:	.long	BSC_BASE + 0x18
CS6ABCR_A:	.long	BSC_BASE + 0x1C
CS6BBCR_A:	.long	BSC_BASE + 0x20
CS0WCR_A:	.long	BSC_BASE + 0x24
CS2WCR_A:	.long	BSC_BASE + 0x28
CS3WCR_A:	.long	BSC_BASE + 0x2C
CS4WCR_A:	.long	BSC_BASE + 0x30
CS5AWCR_A:	.long	BSC_BASE + 0x34
CS5BWCR_A:	.long	BSC_BASE + 0x38
CS6AWCR_A:	.long	BSC_BASE + 0x3C
CS6BWCR_A:	.long	BSC_BASE + 0x40
SDCR_A:		.long	BSC_BASE + 0x44
RTCSR_A:	.long	BSC_BASE + 0x48
RTCNT_A:	.long	BSC_BASE + 0x4C
RTCOR_A:	.long	BSC_BASE + 0x50
SDMR3_A:	.long	BSC_BASE + 0x58C0

CMNCR_D:	.long	0x00000010
CS0BCR_D:	.long	0x36DB0400
CS2BCR_D:	.long	0x36DB0400
CS3BCR_D:	.long	0x36DB4600
CS4BCR_D:	.long	0x36DB0400
CS5ABCR_D:	.long	0x36DB0400
CS5BBCR_D:	.long	0x36DB0200
CS6ABCR_D:	.long	0x36DB0400
CS6BBCR_D:	.long	0x36DB0400
CS0WCR_D:	.long	0x00000B01
CS2WCR_D:	.long	0x00000500
CS3WCR_D:	.long	0x00006D1B
CS4WCR_D:	.long	0x00000500
CS5AWCR_D:	.long	0x00000500
CS5BWCR_D:	.long	0x00000500
CS6AWCR_D:	.long	0x00000500
CS6BWCR_D:	.long	0x00000500
SDCR_D1:	.long	0x00000011
RTCSR_D:	.long	0xA55A0010
RTCNT_D:	.long	0xA55A001F
RTCOR_D:	.long	0xA55A001F
SDMR3_D:	.long	0x0000
SDCR_D2:	.long	0x00000811

#define PFC_BASE	0xA4050100
PCCR_A:		.long	PFC_BASE + 0x04
PDCR_A:		.long	PFC_BASE + 0x06
PECR_A:		.long	PFC_BASE + 0x08
PGCR_A:		.long	PFC_BASE + 0x0C
PHCR_A:		.long	PFC_BASE + 0x0E
PPCR_A:		.long	PFC_BASE + 0x18
PTCR_A:		.long	PFC_BASE + 0x1E
PVCR_A:		.long	PFC_BASE + 0x22
PSELA_A:	.long	PFC_BASE + 0x24

PCCR_D:		.long	0x0000
PDCR_D:		.long	0x0000
PECR_D:		.long	0x0000
PGCR_D:		.long	0x0000
PHCR_D:		.long	0x0000
PPCR_D:		.long	0x00AA
PTCR_D:		.long	0x0280
PVCR_D:		.long	0x0000
PSELA_D:	.long	0x0000

CCR_A:		.long	0xFFFFFFEC
!CCR_D:		.long	0x0000000D
CCR_D:		.long	0x0000000B

LED_A:		.long	0xB6800000
LED_D:		.long	0xFF
