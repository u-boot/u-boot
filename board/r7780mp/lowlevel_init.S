/*
 * Copyright (C) 2007,2008 Nobuhiro Iwamatsu
 *
 * u-boot/board/r7780mp/lowlevel_init.S
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

	mov.l	CCR_A, r1	/* Address of Cache Control Register */
	mov.l	CCR_D, r0	/* Instruction Cache Invalidate */
	mov.l	r0, @r1

	mov.l	FRQCR_A, r1	/* Frequency control register */
	mov.l	FRQCR_D, r0
	mov.l	r0, @r1

	/* pin_multi_setting */
	mov.l   BBG_PMMR_A,r1
	mov.l   BBG_PMMR_D_PMSR1,r0
	mov.l   r0,@r1

	mov.l   BBG_PMSR1_A,r1
	mov.l   BBG_PMSR1_D,r0
	mov.l   r0,@r1

	mov.l   BBG_PMMR_A,r1
	mov.l   BBG_PMMR_D_PMSR2,r0
	mov.l   r0,@r1

	mov.l   BBG_PMSR2_A,r1
	mov.l   BBG_PMSR2_D,r0
	mov.l   r0,@r1

	mov.l   BBG_PMMR_A,r1
	mov.l   BBG_PMMR_D_PMSR3,r0
	mov.l   r0,@r1

	mov.l   BBG_PMSR3_A,r1
	mov.l   BBG_PMSR3_D,r0
	mov.l   r0,@r1

	mov.l   BBG_PMMR_A,r1
	mov.l   BBG_PMMR_D_PMSR4,r0
	mov.l   r0,@r1

	mov.l   BBG_PMSR4_A,r1
	mov.l   BBG_PMSR4_D,r0
	mov.l   r0,@r1

	mov.l   BBG_PMMR_A,r1
	mov.l   BBG_PMMR_D_PMSRG,r0
	mov.l   r0,@r1

	mov.l   BBG_PMSRG_A,r1
	mov.l   BBG_PMSRG_D,r0
	mov.l   r0,@r1

	/* cpg_setting */
	mov.l   FRQCR_A,r1
	mov.l   FRQCR_D,r0
	mov.l   r0,@r1

	mov.l   DLLCSR_A,r1
	mov.l   DLLCSR_D,r0
	mov.l   r0,@r1

	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	/* wait 200us */
	mov.l   REPEAT0_R3,r3
	mov     #0,r2
repeat0:
	add     #1,r2
	cmp/hs  r3,r2
	bf      repeat0
	nop

	/* bsc_setting */
	mov.l	MMSELR_A,r1
	mov.l	MMSELR_D,r0
	mov.l	r0,@r1

	mov.l	BCR_A,r1
	mov.l	BCR_D,r0
	mov.l	r0,@r1

	mov.l	CS0BCR_A,r1
	mov.l	CS0BCR_D,r0
	mov.l	r0,@r1

	mov.l	CS1BCR_A,r1
	mov.l	CS1BCR_D,r0
	mov.l	r0,@r1

	mov.l	CS2BCR_A,r1
	mov.l	CS2BCR_D,r0
	mov.l	r0,@r1

	mov.l	CS4BCR_A,r1
	mov.l	CS4BCR_D,r0
	mov.l	r0,@r1

	mov.l	CS5BCR_A,r1
	mov.l	CS5BCR_D,r0
	mov.l	r0,@r1

	mov.l	CS6BCR_A,r1
	mov.l	CS6BCR_D,r0
	mov.l	r0,@r1

	mov.l	CS0WCR_A,r1
	mov.l	CS0WCR_D,r0
	mov.l	r0,@r1

	mov.l	CS1WCR_A,r1
	mov.l	CS1WCR_D,r0
	mov.l	r0,@r1

	mov.l	CS2WCR_A,r1
	mov.l	CS2WCR_D,r0
	mov.l	r0,@r1

	mov.l	CS4WCR_A,r1
	mov.l	CS4WCR_D,r0
	mov.l	r0,@r1

	mov.l	CS5WCR_A,r1
	mov.l	CS5WCR_D,r0
	mov.l	r0,@r1

	mov.l	CS6WCR_A,r1
	mov.l	CS6WCR_D,r0
	mov.l	r0,@r1

	mov.l	CS5PCR_A,r1
	mov.l	CS5PCR_D,r0
	mov.l	r0,@r1

	mov.l	CS6PCR_A,r1
	mov.l	CS6PCR_D,r0
	mov.l	r0,@r1

	/* ddr_setting */
	/* wait 200us */
	mov.l   REPEAT0_R3,r3
	mov     #0,r2
repeat1:
	add     #1,r2
	cmp/hs  r3,r2
	bf      repeat1
	nop

	mov.l   MIM_U_A,r0
	mov.l   MIM_U_D,r1
	synco
	mov.l   r1,@r0
	synco

	mov.l	MIM_L_A,r0
	mov.l	MIM_L_D0,r1
	synco
	mov.l	r1,@r0
	synco

	mov.l   STR_L_A,r0
	mov.l   STR_L_D,r1
	synco
	mov.l   r1,@r0
	synco

	mov.l   SDR_L_A,r0
	mov.l   SDR_L_D,r1
	synco
	mov.l   r1,@r0
	synco

	nop
	nop
	nop
	nop

	mov.l   SCR_L_A,r0
	mov.l   SCR_L_D0,r1
	synco
	mov.l   r1,@r0
	synco

	mov.l   SCR_L_A,r0
	mov.l   SCR_L_D1,r1
	synco
	mov.l   r1,@r0
	synco

	nop
	nop
	nop

	mov.l   EMRS_A,r0
	mov.l   EMRS_D,r1
	synco
	mov.l   r1,@r0
	synco

	nop
	nop
	nop

	mov.l   MRS1_A,r0
	mov.l   MRS1_D,r1
	synco
	mov.l   r1,@r0
	synco

	nop
	nop
	nop

	mov.l   SCR_L_A,r0
	mov.l   SCR_L_D2,r1
	synco
	mov.l   r1,@r0
	synco

	nop
	nop
	nop

	mov.l   SCR_L_A,r0
	mov.l   SCR_L_D3,r1
	synco
	mov.l   r1,@r0
	synco

	nop
	nop
	nop

	mov.l   SCR_L_A,r0
	mov.l   SCR_L_D4,r1
	synco
	mov.l   r1,@r0
	synco

	nop
	nop
	nop

	mov.l   MRS2_A,r0
	mov.l   MRS2_D,r1
	synco
	mov.l   r1,@r0
	synco

	nop
	nop
	nop

	mov.l   SCR_L_A,r0
	mov.l   SCR_L_D5,r1
	synco
	mov.l   r1,@r0
	synco

	/* wait 200us */
	mov.l   REPEAT0_R1,r3
	mov     #0,r2
repeat2:
	add     #1,r2
	cmp/hs  r3,r2
	bf      repeat2

	synco

	mov.l   MIM_L_A,r0
	mov.l   MIM_L_D1,r1
	synco
	mov.l   r1,@r0
	synco

	rts
	nop
	.align	4

RWTCSR_D_1:				.word	0xA507
RWTCSR_D_2:				.word	0xA507
RWTCNT_D:				.word	0x5A00

BBG_PMMR_A:			.long	0xFF800010
BBG_PMSR1_A:			.long	0xFF800014
BBG_PMSR2_A:			.long	0xFF800018
BBG_PMSR3_A:			.long	0xFF80001C
BBG_PMSR4_A:			.long	0xFF800020
BBG_PMSRG_A:			.long	0xFF800024

BBG_PMMR_D_PMSR1:       .long	0xffffbffd
BBG_PMSR1_D:            .long	0x00004002
BBG_PMMR_D_PMSR2:       .long	0xfc21a7ff
BBG_PMSR2_D:            .long	0x03de5800
BBG_PMMR_D_PMSR3:       .long	0xfffffff8
BBG_PMSR3_D:            .long	0x00000007
BBG_PMMR_D_PMSR4:       .long	0xdffdfff9
BBG_PMSR4_D:            .long	0x20020006
BBG_PMMR_D_PMSRG:       .long	0xffffffff
BBG_PMSRG_D:            .long	0x00000000

FRQCR_A:				.long	FRQCR
DLLCSR_A:				.long	0xffc40010
FRQCR_D:				.long	0x40233035
DLLCSR_D:				.long	0x00000000

/* for DDR-SDRAM */
MIM_U_A:				.long	MIM_1
MIM_L_A:				.long	MIM_2
SCR_U_A:				.long	SCR_1
SCR_L_A:				.long	SCR_2
STR_U_A:				.long	STR_1
STR_L_A:				.long	STR_2
SDR_U_A:				.long	SDR_1
SDR_L_A:				.long	SDR_2

EMRS_A:					.long	0xFEC02000
MRS1_A:					.long	0xFEC00B08
MRS2_A:					.long	0xFEC00308

MIM_U_D:				.long	0x00004000
MIM_L_D0:				.long	0x03e80009
MIM_L_D1:				.long	0x03e80209
SCR_L_D0:				.long	0x3
SCR_L_D1:				.long	0x2
SCR_L_D2:				.long	0x2
SCR_L_D3:				.long	0x4
SCR_L_D4:				.long	0x4
SCR_L_D5:				.long	0x0
STR_L_D:				.long	0x000f0000
SDR_L_D:				.long	0x00000400
EMRS_D:					.long	0x0
MRS1_D:					.long	0x0
MRS2_D:					.long	0x0

/* Cache Controller */
CCR_A:		.long	CCR
MMUCR_A:	.long	MMUCR
RWTCNT_A:	.long	WTCNT

CCR_D:		.long	0x0000090b
CCR_D_2:	.long	0x00000103
MMUCR_D:	.long	0x00000004
MSTPCR0_D:	.long	0x00001001
MSTPCR2_D:	.long	0xffffffff

/* local Bus State Controller */
MMSELR_A:   .long   MMSELR
BCR_A:      .long   BCR
CS0BCR_A:   .long   CS0BCR
CS1BCR_A:   .long   CS1BCR
CS2BCR_A:   .long   CS2BCR
CS4BCR_A:   .long   CS4BCR
CS5BCR_A:   .long   CS5BCR
CS6BCR_A:   .long   CS6BCR
CS0WCR_A:   .long   CS0WCR
CS1WCR_A:   .long   CS1WCR
CS2WCR_A:   .long   CS2WCR
CS4WCR_A:   .long   CS4WCR
CS5WCR_A:   .long   CS5WCR
CS6WCR_A:   .long   CS6WCR
CS5PCR_A:   .long   CS5PCR
CS6PCR_A:   .long   CS6PCR

MMSELR_D:		.long	0xA5A50003
BCR_D:			.long	0x00000000
CS0BCR_D:		.long	0x77777770
CS1BCR_D:		.long	0x77777670
CS2BCR_D:		.long	0x77777770
CS4BCR_D:		.long	0x77777770
CS5BCR_D:		.long	0x77777670
CS6BCR_D:		.long	0x77777770
CS0WCR_D:		.long	0x00020006
CS1WCR_D:		.long	0x00232304
CS2WCR_D:		.long	0x7777770F
CS4WCR_D:		.long	0x7777770F
CS5WCR_D:		.long	0x00101006
CS6WCR_D:		.long	0x77777703
CS5PCR_D:		.long	0x77000000
CS6PCR_D:		.long	0x77000000

REPEAT0_R3:	.long   0x00002000
REPEAT0_R1:	.long   0x0000200
