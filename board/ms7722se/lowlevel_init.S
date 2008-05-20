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

/*
 *  Board specific low level init code, called _very_ early in the
 *  startup sequence. Relocation to SDRAM has not happened yet, no
 *  stack is available, bss section has not been initialised, etc.
 *
 *  (Note: As no stack is available, no subroutines can be called...).
 */

	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:

	mov.l	CCR_A, r1	! Address of Cache Control Register
	mov.l	CCR_D, r0	! Instruction Cache Invalidate
	mov.l	r0, @r1

	mov.l	MMUCR_A, r1	! Address of MMU Control Register
	mov.l	MMUCR_D, r0	! TI == TLB Invalidate bit
	mov.l	r0, @r1

	mov.l	MSTPCR0_A, r1	! Address of Power Control Register 0
	mov.l	MSTPCR0_D, r0	!
	mov.l	r0, @r1

	mov.l	MSTPCR2_A, r1	! Address of Power Control Register 2
	mov.l	MSTPCR2_D, r0	!
	mov.l	r0, @r1

	mov.l	SBSCR_A, r1	!
	mov.w	SBSCR_D, r0	!
	mov.w	r0, @r1

	mov.l	PSCR_A, r1	!
	mov.w	PSCR_D, r0	!
	mov.w	r0, @r1

!	mov.l	RWTCSR_A, r1	! 0xA4520004 (Watchdog Control / Status Register)
!	mov.w	RWTCSR_D_1, r0	! 0xA507 -> timer_STOP/WDT_CLK=max
!	mov.w	r0, @r1

	mov.l	RWTCNT_A, r1	! 0xA4520000 (Watchdog Count Register)
	mov.w	RWTCNT_D, r0	! 0x5A00 -> Clear
	mov.w	r0, @r1

	mov.l	RWTCSR_A, r1	! 0xA4520004 (Watchdog Control / Status Register)
	mov.w	RWTCSR_D_2, r0	! 0xA504 -> timer_STOP/CLK=500ms
	mov.w	r0, @r1

	mov.l	FRQCR_A, r1		! 0xA4150000 Frequency control register
	mov.l	FRQCR_D, r0	!
	mov.l	r0, @r1

	mov.l	CCR_A, r1		! Address of Cache Control Register
	mov.l	CCR_D_2, r0	! ??
	mov.l	r0, @r1

bsc_init:

	mov.l	PSELA_A, r1
	mov.w	PSELA_D, r0
	mov.w	r0, @r1

	mov.l	DRVCR_A, r1
	mov.w	DRVCR_D, r0
	mov.w	r0, @r1

	mov.l	PCCR_A, r1
	mov.w	PCCR_D, r0
	mov.w	r0, @r1

	mov.l	PECR_A, r1
	mov.w	PECR_D, r0
	mov.w	r0, @r1

	mov.l	PJCR_A, r1
	mov.w	PJCR_D, r0
	mov.w	r0, @r1

	mov.l	PXCR_A, r1
	mov.w	PXCR_D, r0
	mov.w	r0, @r1

	mov.l	CMNCR_A, r1	! CMNCR address -> R1
	mov.l	CMNCR_D, r0	! CMNCR data    -> R0
	mov.l	r0, @r1		! CMNCR set

	mov.l	CS0BCR_A, r1	! CS0BCR address -> R1
	mov.l	CS0BCR_D, r0	! CS0BCR data    -> R0
	mov.l	r0, @r1		! CS0BCR set

	mov.l	CS2BCR_A, r1	! CS2BCR address -> R1
	mov.l	CS2BCR_D, r0	! CS2BCR data    -> R0
	mov.l	r0, @r1		! CS2BCR set

	mov.l	CS4BCR_A, r1	! CS4BCR address -> R1
	mov.l	CS4BCR_D, r0	! CS4BCR data    -> R0
	mov.l	r0, @r1		! CS4BCR set

	mov.l	CS5ABCR_A, r1	! CS5ABCR address -> R1
	mov.l	CS5ABCR_D, r0	! CS5ABCR data    -> R0
	mov.l	r0, @r1		! CS5ABCR set

	mov.l	CS5BBCR_A, r1	! CS5BBCR address -> R1
	mov.l	CS5BBCR_D, r0	! CS5BBCR data    -> R0
	mov.l	r0, @r1		! CS5BBCR set

	mov.l	CS6ABCR_A, r1	! CS6ABCR address -> R1
	mov.l	CS6ABCR_D, r0	! CS6ABCR data    -> R0
	mov.l	r0, @r1		! CS6ABCR set

	mov.l	CS0WCR_A, r1	! CS0WCR address -> R1
	mov.l	CS0WCR_D, r0	! CS0WCR data    -> R0
	mov.l	r0, @r1		! CS0WCR set

	mov.l	CS2WCR_A, r1	! CS2WCR address -> R1
	mov.l	CS2WCR_D, r0	! CS2WCR data    -> R0
	mov.l	r0, @r1		! CS2WCR set

	mov.l	CS4WCR_A, r1	! CS4WCR address -> R1
	mov.l	CS4WCR_D, r0	! CS4WCR data    -> R0
	mov.l	r0, @r1		! CS4WCR set

	mov.l	CS5AWCR_A, r1	! CS5AWCR address -> R1
	mov.l	CS5AWCR_D, r0	! CS5AWCR data    -> R0
	mov.l	r0, @r1		! CS5AWCR set

	mov.l	CS5BWCR_A, r1	! CS5BWCR address -> R1
	mov.l	CS5BWCR_D, r0	! CS5BWCR data    -> R0
	mov.l	r0, @r1		! CS5BWCR set

	mov.l	CS6AWCR_A, r1	! CS6AWCR address -> R1
	mov.l	CS6AWCR_D, r0	! CS6AWCR data    -> R0
	mov.l	r0, @r1		! CS6AWCR set

	! SDRAM initialization
	mov.l	SDCR_A, r1	! SB_SDCR address -> R1
	mov.l	SDCR_D, r0	! SB_SDCR data    -> R0
	mov.l	r0, @r1		! SB_SDCR set

	mov.l	SDWCR_A, r1	! SB_SDWCR address -> R1
	mov.l	SDWCR_D, r0	! SB_SDWCR data    -> R0
	mov.l	r0, @r1		! SB_SDWCR set

	mov.l	SDPCR_A, r1	! SB_SDPCR address -> R1
	mov.l	SDPCR_D, r0	! SB_SDPCR data    -> R0
	mov.l	r0, @r1		! SB_SDPCR set

	mov.l	RTCOR_A, r1	! SB_RTCOR address -> R1
	mov.l	RTCOR_D, r0	! SB_RTCOR data    -> R0
	mov.l	r0, @r1		! SB_RTCOR set

	mov.l	RTCSR_A, r1	! SB_RTCSR address -> R1
	mov.l	RTCSR_D, r0	! SB_RTCSR data    -> R0
	mov.l	r0, @r1		! SB_RTCSR set

	mov.l	SDMR3_A, r1	! SDMR3 address -> R1
	mov	#0x00, r0	! SDMR3 data    -> R0
	mov.b	r0, @r1		! SDMR3 set

	! BL bit off (init = ON)  (?!?)

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

PSELA_A:	.long   0xa405014E
PSELA_D:	.word   0x0A10
	.align 2

DRVCR_A:	.long   0xa405018A
DRVCR_D:	.word   0x0554
	.align 2

PCCR_A:		.long   0xa4050104
PCCR_D:		.word   0x8800
	.align 2

PECR_A:		.long   0xa4050108
PECR_D:		.word   0x0000
	.align 2

PJCR_A:		.long   0xa4050110
PJCR_D:		.word   0x1000
	.align 2

PXCR_A:		.long   0xa4050148
PXCR_D:		.word   0x0AAA
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

	.align	1

SBSCR_D:	.word	0x0040
PSCR_D:		.word	0x0000
RWTCSR_D_1:	.word	0xA507
RWTCSR_D_2:	.word	0xA507
RWTCNT_D:	.word	0x5A00

SR_MASK_D:	.long	0xEFFFFF0F
