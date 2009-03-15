/*
 * Copyright (C) 2008 Renesas Solutions Corp.
 * Copyright (C) 2008 Nobuhiro Iwamatsu <iwamatsu.nobuhiro@renesas.com>
 *
 * board/ap325rxa/lowlevel_init.S
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
#include <asm/processor.h>
#include <asm/macro.h>

/*
 * Board specific low level init code, called _very_ early in the
 * startup sequence. Relocation to SDRAM has not happened yet, no
 * stack is available, bss section has not been initialised, etc.
 *
 * (Note: As no stack is available, no subroutines can be called...).
 */

	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:
	write16	DRVCRA_A, DRVCRA_D

	write16	DRVCRB_A, DRVCRB_D

	write16	RWTCSR_A, RWTCSR_D1

	write16	RWTCNT_A, RWTCNT_D

	write16	RWTCSR_A, RWTCSR_D2

	write32	FRQCR_A, FRQCR_D

	write32	CMNCR_A, CMNCR_D

	write32	CS0BCR_A, CS0BCR_D

	write32	CS4BCR_A, CS4BCR_D

	write32	CS5ABCR_A, CS5ABCR_D

	write32	CS5BBCR_A, CS5BBCR_D

	write32	CS6ABCR_A, CS6ABCR_D

	write32	CS6BBCR_A, CS6BBCR_D

	write32	CS0WCR_A, CS0WCR_D

	write32	CS4WCR_A, CS4WCR_D

	write32	CS5AWCR_A, CS5AWCR_D

	write32	CS5BWCR_A, CS5BWCR_D

	write32	CS6AWCR_A, CS6AWCR_D

	write32	CS6BWCR_A, CS6BWCR_D

	write32	SBSC_SDCR_A, SBSC_SDCR_D1

	write32	SBSC_SDWCR_A, SBSC_SDWCR_D

	write32	SBSC_SDPCR_A, SBSC_SDPCR_D

	write32	SBSC_RTCSR_A, SBSC_RTCSR_D

	write32	SBSC_RTCNT_A, SBSC_RTCNT_D

	write32	SBSC_RTCOR_A, SBSC_RTCOR_D

	write8	SBSC_SDMR3_A1, SBSC_SDMR3_D

	write8	SBSC_SDMR3_A2, SBSC_SDMR3_D

	mov.l	SLEEP_CNT, r1
2:	tst	r1, r1
	nop
	bf/s	2b
	dt	r1

	write8	SBSC_SDMR3_A3, SBSC_SDMR3_D

	write32	SBSC_SDCR_A, SBSC_SDCR_D2

	write32	CCR_A, CCR_D

	! BL bit off (init = ON) (?!?)

	stc	sr, r0				! BL bit off(init=ON)
	mov.l	SR_MASK_D, r1
	and	r1, r0
	ldc	r0, sr

	rts
	 mov	#0, r0

	.align	2

DRVCRA_A:	.long	DRVCRA
DRVCRB_A:	.long	DRVCRB
DRVCRA_D:	.long	0x4555
DRVCRB_D:	.long	0x0005

RWTCSR_A:	.long	RWTCSR
RWTCNT_A:	.long	RWTCNT
FRQCR_A:	.long	FRQCR
RWTCSR_D1:	.long	0xa507
RWTCSR_D2:	.long	0xa504
RWTCNT_D:	.long	0x5a00
FRQCR_D:	.long	0x0b04474a

SBSC_SDCR_A:	.long	SBSC_SDCR
SBSC_SDWCR_A:	.long	SBSC_SDWCR
SBSC_SDPCR_A:	.long	SBSC_SDPCR
SBSC_RTCSR_A:	.long	SBSC_RTCSR
SBSC_RTCNT_A:	.long	SBSC_RTCNT
SBSC_RTCOR_A:	.long	SBSC_RTCOR
SBSC_SDMR3_A1:	.long	0xfe510000
SBSC_SDMR3_A2:	.long	0xfe500242
SBSC_SDMR3_A3:	.long	0xfe5c0042

SBSC_SDCR_D1:	.long	0x92810112
SBSC_SDCR_D2:	.long	0x92810912
SBSC_SDWCR_D:	.long	0x05162482
SBSC_SDPCR_D:	.long	0x00300087
SBSC_RTCSR_D:	.long	0xa55a0212
SBSC_RTCNT_D:	.long	0xa55a0000
SBSC_RTCOR_D:	.long	0xa55a0040
SBSC_SDMR3_D:	.long	0x00

CMNCR_A:	.long	CMNCR
CS0BCR_A:	.long	CS0BCR
CS4BCR_A:	.long	CS4BCR
CS5ABCR_A:	.long	CS5ABCR
CS5BBCR_A:	.long	CS5BBCR
CS6ABCR_A:	.long	CS6ABCR
CS6BBCR_A:	.long	CS6BBCR
CS0WCR_A:	.long	CS0WCR
CS4WCR_A:	.long	CS4WCR
CS5AWCR_A:	.long	CS5AWCR
CS5BWCR_A:	.long	CS5BWCR
CS6AWCR_A:	.long	CS6AWCR
CS6BWCR_A:	.long	CS6BWCR

CMNCR_D:	.long	0x00000013
CS0BCR_D:	.long	0x24920400
CS4BCR_D:	.long	0x24920400
CS5ABCR_D:	.long	0x24920400
CS5BBCR_D:	.long	0x7fff0600
CS6ABCR_D:	.long	0x24920400
CS6BBCR_D:	.long	0x24920600
CS0WCR_D:	.long	0x00000480
CS4WCR_D:	.long	0x00000480
CS5AWCR_D:	.long	0x00000380
CS5BWCR_D:	.long	0x00000080
CS6AWCR_D:	.long	0x00000300
CS6BWCR_D:	.long	0x00000540

CCR_A:		.long	0xff00001c
CCR_D:		.long	0x0000090d

SLEEP_CNT:	.long	0x00000800
SR_MASK_D:	.long	0xEFFFFF0F
