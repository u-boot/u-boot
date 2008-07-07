/*
 * Copyright (C) 2008 Renesas Solutions Corp.
 * Copyright (C) 2008 Nobuhiro Iwamatsu <iwamatsu.nobuhiro@renesas.com>
 * Copyright (C) 2007 Kenati Technologies, Inc.
 *
 * board/sh7763rdp/lowlevel_init.S
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

	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:

	mov.l   WDTCSR_A, r1	/* Watchdog Control / Status Register */
	mov.l   WDTCSR_D, r0
	mov.l   r0, @r1

	mov.l   WDTST_A, r1		/* Watchdog Stop Time Register */
	mov.l   WDTST_D, r0
	mov.l   r0, @r1

	mov.l   WDTBST_A, r1	/* 0xFFCC0008 (Watchdog Base Stop Time Register */
	mov.l   WDTBST_D, r0
	mov.l   r0, @r1

	mov.l	CCR_A, r1		/* Address of Cache Control Register */
	mov.l	CCR_CACHE_ICI_D, r0	/* Instruction Cache Invalidate */
	mov.l	r0, @r1

	mov.l	MMUCR_A, r1		/* Address of MMU Control Register */
	mov.l	MMU_CONTROL_TI_D, r0	/* TI == TLB Invalidate bit */
	mov.l	r0, @r1

	mov.l	MSTPCR0_A, r1	/* Address of Power Control Register 0 */
	mov.l	MSTPCR0_D, r0
	mov.l	r0, @r1

	mov.l	MSTPCR1_A, r1	/*i Address of Power Control Register 1 */
	mov.l	MSTPCR1_D, r0
	mov.l	r0, @r1

	mov.l	RAMCR_A,r1
	mov.l	RAMCR_D,r0
	mov.l	r0, @r1

	mov.l	MMSELR_A,r1
	mov.l	MMSELR_D,r0
	synco
	mov.l	r0, @r1

	mov.l	@r1,r2		/* execute two reads after setting MMSELR*/
	mov.l	@r1,r2
	synco

	/* issue memory read */
	mov.l   DDRSD_START_A,r1	/* memory address to read*/
	mov.l   @r1,r0
	synco

	mov.l	MIM8_A,r1
	mov.l	MIM8_D,r0
	mov.l	r0,@r1

	mov.l	MIMC_A,r1
	mov.l	MIMC_D1,r0
	mov.l	r0,@r1

	mov.l	STRC_A,r1
	mov.l	STRC_D,r0
	mov.l	r0,@r1

	mov.l	SDR4_A,r1
	mov.l	SDR4_D,r0
	mov.l	r0,@r1

	mov.l	MIMC_A,r1
	mov.l	MIMC_D2,r0
	mov.l	r0,@r1

	nop
	nop
	nop

	mov.l	SCR4_A,r1
	mov.l	SCR4_D3,r0
	mov.l	r0,@r1

	mov.l	SCR4_A,r1
	mov.l	SCR4_D2,r0
	mov.l	r0,@r1

	mov.l	SDMR02000_A,r1
	mov.l	SDMR02000_D,r0
	mov.l	r0,@r1

	mov.l	SDMR00B08_A,r1
	mov.l	SDMR00B08_D,r0
	mov.l	r0,@r1

	mov.l	SCR4_A,r1
	mov.l	SCR4_D2,r0
	mov.l	r0,@r1

	mov.l	SCR4_A,r1
	mov.l	SCR4_D4,r0
	mov.l	r0,@r1

	nop
	nop
	nop
	nop

	mov.l	SCR4_A,r1
	mov.l	SCR4_D4,r0
	mov.l	r0,@r1

	nop
	nop
	nop
	nop

	mov.l	SDMR00308_A,r1
	mov.l	SDMR00308_D,r0
	mov.l	r0,@r1

	mov.l	MIMC_A,r1
	mov.l	MIMC_D3,r0
	mov.l	r0,@r1

	mov.l	SCR4_A,r1
	mov.l	SCR4_D1,r0
	mov.l	DELAY60_D,r3

delay_loop_60:
	mov.l	r0,@r1
	dt	r3
	bf	delay_loop_60
	nop

	mov.l   CCR_A, r1	/* Address of Cache Control Register */
	mov.l   CCR_CACHE_D_2, r0
	mov.l   r0, @r1

bsc_init:
	mov.l	BCR_A, r1
	mov.l	BCR_D, r0
	mov.l	r0, @r1

	mov.l	CS0BCR_A, r1
	mov.l	CS0BCR_D, r0
	mov.l	r0, @r1

	mov.l	CS1BCR_A,r1
	mov.l	CS1BCR_D,r0
	mov.l	r0,@r1

	mov.l	CS2BCR_A, r1
	mov.l	CS2BCR_D, r0
	mov.l	r0, @r1

	mov.l	CS4BCR_A, r1
	mov.l	CS4BCR_D, r0
	mov.l	r0, @r1

	mov.l	CS5BCR_A, r1
	mov.l	CS5BCR_D, r0
	mov.l	r0, @r1

	mov.l	CS6BCR_A, r1
	mov.l	CS6BCR_D, r0
	mov.l	r0, @r1

	mov.l	CS0WCR_A, r1
	mov.l	CS0WCR_D, r0
	mov.l	r0, @r1

	mov.l	CS1WCR_A, r1
	mov.l	CS1WCR_D, r0
	mov.l	r0, @r1

	mov.l	CS2WCR_A, r1
	mov.l	CS2WCR_D, r0
	mov.l	r0, @r1

	mov.l	CS4WCR_A, r1
	mov.l	CS4WCR_D, r0
	mov.l	r0, @r1

	mov.l	CS5WCR_A, r1
	mov.l	CS5WCR_D, r0
	mov.l	r0, @r1

	mov.l	CS6WCR_A, r1
	mov.l	CS6WCR_D, r0
	mov.l	r0, @r1

	mov.l	CS5PCR_A, r1
	mov.l	CS5PCR_D, r0
	mov.l	r0, @r1

	mov.l	CS6PCR_A, r1
	mov.l	CS6PCR_D, r0
	mov.l	r0, @r1

	mov.l	DELAY200_D,r3

delay_loop_200:
	dt	r3
	bf	delay_loop_200
	nop

	mov.l	PSEL0_A,r1
	mov.l	PSEL0_D,r0
	mov.w	r0,@r1

	mov.l	PSEL1_A,r1
	mov.l	PSEL1_D,r0
	mov.w	r0,@r1

	mov.l	ICR0_A,r1
	mov.l	ICR0_D,r0
	mov.l	r0,@r1

	stc sr, r0	/* BL bit off(init=ON) */
	mov.l   SR_MASK_D, r1
	and r1, r0
	ldc r0, sr

	rts
	nop

	.align	2

DELAY60_D:	.long	60
DELAY200_D:	.long	17800

CCR_A:		.long	0xFF00001C
MMUCR_A:	.long	0xFF000010
RAMCR_A:	.long	0xFF000074

/* Low power mode control */
MSTPCR0_A:	.long	0xFFC80030
MSTPCR1_A:	.long	0xFFC80038

/* RWBT */
WDTST_A:	.long	0xFFCC0000
WDTCSR_A:	.long	0xFFCC0004
WDTBST_A:	.long	0xFFCC0008

/* BSC */
MMSELR_A:	.long	0xFE600020
BCR_A:		.long	0xFF801000
CS0BCR_A:	.long	0xFF802000
CS1BCR_A:	.long	0xFF802010
CS2BCR_A:	.long	0xFF802020
CS4BCR_A:	.long	0xFF802040
CS5BCR_A:	.long	0xFF802050
CS6BCR_A:	.long	0xFF802060
CS0WCR_A:	.long	0xFF802008
CS1WCR_A:	.long	0xFF802018
CS2WCR_A:	.long	0xFF802028
CS4WCR_A:	.long	0xFF802048
CS5WCR_A:	.long	0xFF802058
CS6WCR_A:	.long	0xFF802068
CS5PCR_A:	.long	0xFF802070
CS6PCR_A:	.long	0xFF802080
DDRSD_START_A:	.long	0xAC000000

/* INTC */
ICR0_A:		.long	0xFFD00000

/* DDR I/F */
MIM8_A:		.long	0xFE800008
MIMC_A:		.long	0xFE80000C
SCR4_A:		.long	0xFE800014
STRC_A:		.long	0xFE80001C
SDR4_A:		.long	0xFE800034
SDMR00308_A:	.long	0xFE900308
SDMR00B08_A:	.long	0xFE900B08
SDMR02000_A:	.long	0xFE902000

/* GPIO */
PSEL0_A:	.long	0xFFEF0070
PSEL1_A:	.long	0xFFEF0072

CCR_CACHE_ICI_D:.long	0x00000800
CCR_CACHE_D_2:	.long	0x00000103
MMU_CONTROL_TI_D:.long	0x00000004
RAMCR_D:	.long	0x00000200
MSTPCR0_D:	.long	0x00000000
MSTPCR1_D:	.long	0x00000000

MMSELR_D:	.long	0xa5a50000
BCR_D:		.long	0x00000000
CS0BCR_D:	.long	0x77777770
CS1BCR_D:	.long	0x77777670
CS2BCR_D:	.long	0x77777670
CS4BCR_D:	.long	0x77777670
CS5BCR_D:	.long	0x77777670
CS6BCR_D:	.long	0x77777670
CS0WCR_D:	.long	0x7777770F
CS1WCR_D:	.long   0x22000002
CS2WCR_D:	.long	0x7777770F
CS4WCR_D:	.long	0x7777770F
CS5WCR_D:	.long	0x7777770F
CS6WCR_D:	.long	0x7777770F
CS5PCR_D:	.long	0x77000000
CS6PCR_D:	.long	0x77000000
ICR0_D:		.long	0x00E00000
MIM8_D:		.long	0x00000000
MIMC_D1:	.long	0x01d10008
MIMC_D2:	.long	0x01d10009
MIMC_D3:	.long	0x01d10209
SCR4_D1:	.long	0x00000001
SCR4_D2:	.long	0x00000002
SCR4_D3:	.long	0x00000003
SCR4_D4:	.long	0x00000004
STRC_D:		.long	0x000f3980
SDR4_D:		.long	0x00000300
SDMR00308_D:	.long	0x00000000
SDMR00B08_D:	.long	0x00000000
SDMR02000_D:	.long	0x00000000
PSEL0_D:	.long	0x00000001
PSEL1_D:	.long	0x00000244
SR_MASK_D:	.long	0xEFFFFF0F
WDTST_D:	.long	0x5A000FFF
WDTCSR_D:	.long	0xA5000000
WDTBST_D:	.long	0x55000000
