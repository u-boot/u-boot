/*
 * Copyright (C) 2007
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * Copyright (C) 2007
 * Kenati Technologies, Inc.
 *
 * board/ms7722se/lowlevel_init.S
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

	/*
	 * Cache Control Register
	 * Instruction Cache Invalidate
	 */
	write32	CCR_A, CCR_D

	/*
	 * Address of MMU Control Register
	 * TI == TLB Invalidate bit
	 */
	write32	MMUCR_A, MMUCR_D

	/* Address of Power Control Register 0 */
	write32	MSTPCR0_A, MSTPCR0_D

	/* Address of Power Control Register 2 */
	write32	MSTPCR2_A, MSTPCR2_D

	write16	SBSCR_A, SBSCR_D

	write16	PSCR_A, PSCR_D

	/* 0xA4520004 (Watchdog Control / Status Register) */
!	write16	RWTCSR_A, RWTCSR_D_1	/* 0xA507 -> timer_STOP/WDT_CLK=max */

	/* 0xA4520000 (Watchdog Count Register) */
	write16	RWTCNT_A, RWTCNT_D	/*0x5A00 -> Clear */

	/* 0xA4520004 (Watchdog Control / Status Register) */
	write16	RWTCSR_A, RWTCSR_D_2	/* 0xA504 -> timer_STOP/CLK=500ms */

	/* 0xA4150000 Frequency control register */
	write32	FRQCR_A, FRQCR_D

	write32	CCR_A, CCR_D_2

bsc_init:

	write16	PSELA_A, PSELA_D

	write16	DRVCR_A, DRVCR_D

	write16	PCCR_A, PCCR_D

	write16	PECR_A, PECR_D

	write16	PJCR_A, PJCR_D

	write16	PXCR_A, PXCR_D

	write32	CMNCR_A, CMNCR_D

	write32	CS0BCR_A, CS0BCR_D

	write32	CS2BCR_A, CS2BCR_D

	write32	CS4BCR_A, CS4BCR_D

	write32	CS5ABCR_A, CS5ABCR_D

	write32	CS5BBCR_A, CS5BBCR_D

	write32	CS6ABCR_A, CS6ABCR_D

	write32	CS0WCR_A, CS0WCR_D

	write32	CS2WCR_A, CS2WCR_D

	write32	CS4WCR_A, CS4WCR_D

	write32	CS5AWCR_A, CS5AWCR_D

	write32	CS5BWCR_A, CS5BWCR_D

	write32	CS6AWCR_A, CS6AWCR_D

	! SDRAM initialization
	write32	SDCR_A, SDCR_D

	write32	SDWCR_A, SDWCR_D

	write32	SDPCR_A, SDPCR_D

	write32	RTCOR_A, RTCOR_D

	write32	RTCSR_A, RTCSR_D

	write8	SDMR3_A, SDMR3_D

	! BL bit off (init = ON) (?!?)

	stc	sr, r0				! BL bit off(init=ON)
	mov.l	SR_MASK_D, r1
	and	r1, r0
	ldc	r0, sr

	rts
	mov	#0, r0

	.align	2

CCR_A:		.long	CCR
MMUCR_A:	.long	MMUCR
MSTPCR0_A:	.long	MSTPCR0
MSTPCR2_A:	.long	MSTPCR2
SBSCR_A:	.long	SBSCR
PSCR_A:		.long	PSCR
RWTCSR_A:	.long	RWTCSR
RWTCNT_A:	.long	RWTCNT
FRQCR_A:	.long	FRQCR

CCR_D:		.long	0x00000800
CCR_D_2:	.long	0x00000103
MMUCR_D:	.long	0x00000004
MSTPCR0_D:	.long	0x00001001
MSTPCR2_D:	.long	0xffffffff
FRQCR_D:	.long	0x07022538

PSELA_A:	.long	0xa405014E
PSELA_D:	.word	0x0A10
	.align 2

DRVCR_A:	.long	0xa405018A
DRVCR_D:	.word	0x0554
	.align 2

PCCR_A:		.long	0xa4050104
PCCR_D:		.word	0x8800
	.align 2

PECR_A:		.long	0xa4050108
PECR_D:		.word	0x0000
	.align 2

PJCR_A:		.long	0xa4050110
PJCR_D:		.word	0x1000
	.align 2

PXCR_A:		.long	0xa4050148
PXCR_D:		.word	0x0AAA
	.align 2

CMNCR_A:	.long	CMNCR
CMNCR_D:	.long	0x00000013
CS0BCR_A:	.long	CS0BCR		! Flash bank 1
CS0BCR_D:	.long	0x24920400
CS2BCR_A:	.long	CS2BCR		! SRAM
CS2BCR_D:	.long	0x24920400
CS4BCR_A:	.long	CS4BCR		! FPGA, PCMCIA, USB, ext slot
CS4BCR_D:	.long	0x24920400
CS5ABCR_A:	.long	CS5ABCR		! Ext slot
CS5ABCR_D:	.long	0x24920400
CS5BBCR_A:	.long	CS5BBCR		! USB controller
CS5BBCR_D:	.long	0x24920400
CS6ABCR_A:	.long	CS6ABCR		! Ethernet
CS6ABCR_D:	.long	0x24920400

CS0WCR_A:	.long	CS0WCR
CS0WCR_D:	.long	0x00000300
CS2WCR_A:	.long	CS2WCR
CS2WCR_D:	.long	0x00000300
CS4WCR_A:	.long	CS4WCR
CS4WCR_D:	.long	0x00000300
CS5AWCR_A:	.long	CS5AWCR
CS5AWCR_D:	.long	0x00000300
CS5BWCR_A:	.long	CS5BWCR
CS5BWCR_D:	.long	0x00000300
CS6AWCR_A:	.long	CS6AWCR
CS6AWCR_D:	.long	0x00000300

SDCR_A:		.long	SBSC_SDCR
SDCR_D:		.long	0x00020809
SDWCR_A:	.long	SBSC_SDWCR
SDWCR_D:	.long	0x00164d0d
SDPCR_A:	.long	SBSC_SDPCR
SDPCR_D:	.long	0x00000087
RTCOR_A:	.long	SBSC_RTCOR
RTCOR_D:	.long	0xA55A0034
RTCSR_A:	.long	SBSC_RTCSR
RTCSR_D:	.long	0xA55A0010
SDMR3_A:	.long	0xFE500180
SDMR3_D:	.long	0x0

	.align	1

SBSCR_D:	.word	0x0040
PSCR_D:		.word	0x0000
RWTCSR_D_1:	.word	0xA507
RWTCSR_D_2:	.word	0xA507
RWTCNT_D:	.word	0x5A00
	.align	2

SR_MASK_D:	.long	0xEFFFFF0F
