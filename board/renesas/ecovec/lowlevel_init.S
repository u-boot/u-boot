/*
 * Copyright (C) 2011 Renesas Solutions Corp.
 * Copyright (C) 2011 Nobuhiro Iwamatsu <nobuhiro.Iwamatsu.yj@renesas.com>
 *
 * board/renesas/ecovec/lowlevel_init.S
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <asm/processor.h>
#include <asm/macro.h>
#include <configs/ecovec.h>

	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:

	/* jump to CONFIG_ECOVEC_ROMIMAGE_ADDR if bit 1 of PVDR_A */
	mov.l	PVDR_A, r1
	mov.l	PVDR_D, r2
	mov.b	@r1, r0
	tst	r0, r2
	bt	1f
	mov.l	JUMP_A, r1
	jmp	@r1
	nop

1:
	/* Disable watchdog */
	write16 RWTCSR_A, RWTCSR_D

	/* MMU Disable */
	write32 MMUCR_A, MMUCR_D

	/* Setup clocks */
	write32 PLLCR_A, PLLCR_D
	write32 FRQCRA_A, FRQCRA_D
	write32 FRQCRB_A, FRQCRB_D

	wait_timer TIMER_D

	write32 MMSELR_A, MMSELR_D

	/* Srtup BSC */
	write32 CMNCR_A, CMNCR_D
	write32 CS0BCR_A, CS0BCR_D
	write32 CS0WCR_A, CS0WCR_D

	wait_timer TIMER_D

	/* Setup SDRAM */
	write32 DBPDCNT0_A,	DBPDCNT0_D0
	write32 DBCONF_A,	DBCONF_D
	write32 DBTR0_A,	DBTR0_D
	write32 DBTR1_A,	DBTR1_D
	write32 DBTR2_A,	DBTR2_D
	write32 DBTR3_A,	DBTR3_D
	write32 DBKIND_A,	DBKIND_D
	write32 DBCKECNT_A,	DBCKECNT_D

	wait_timer TIMER_D

	write32 DBCMDCNT_A,	DBCMDCNT_D0
	write32 DBMRCNT_A, DBMRCNT_D0
	write32 DBMRCNT_A, DBMRCNT_D1
	write32 DBMRCNT_A, DBMRCNT_D2
	write32 DBMRCNT_A, DBMRCNT_D3
	write32 DBCMDCNT_A, DBCMDCNT_D0
	write32 DBCMDCNT_A, DBCMDCNT_D1
	write32 DBCMDCNT_A, DBCMDCNT_D1
	write32 DBMRCNT_A, DBMRCNT_D4
	write32 DBMRCNT_A, DBMRCNT_D5
	write32 DBMRCNT_A, DBMRCNT_D6

	wait_timer TIMER_D

	write32 DBEN_A, DBEN_D
	write32 DBRFPDN1_A, DBRFPDN1_D
	write32 DBRFPDN2_A, DBRFPDN2_D
	write32 DBCMDCNT_A, DBCMDCNT_D0


	/* Dummy read */
	mov.l DUMMY_A ,r1
	synco
	mov.l @r1, r0
	synco

	mov.l SDRAM_A ,r1
	synco
	mov.l @r1, r0
	synco
	wait_timer TIMER_D

	add #4, r1
	synco
	mov.l @r1, r0
	synco
	wait_timer TIMER_D

	add #4, r1
	synco
	mov.l @r1, r0
	synco
	wait_timer TIMER_D

	add #4, r1
	synco
	mov.l @r1, r0
	synco
	wait_timer TIMER_D

	write32 DBCMDCNT_A, DBCMDCNT_D0
	write32 DBCMDCNT_A, DBCMDCNT_D1
	write32 DBPDCNT0_A, DBPDCNT0_D1
	write32 DBRFPDN0_A, DBRFPDN0_D

	wait_timer TIMER_D

	write32 CCR_A, CCR_D

	stc	sr, r0
	mov.l	SR_MASK_D, r1
	and	r1, r0
	ldc	r0, sr

	rts

	.align	2

PVDR_A:		.long	PVDR
PVDR_D:		.long	0x00000001
JUMP_A:		.long	CONFIG_ECOVEC_ROMIMAGE_ADDR
TIMER_D:	.long	64
RWTCSR_A:	.long	RWTCSR
RWTCSR_D:	.long	0x0000A507
MMUCR_A:	.long	MMUCR
MMUCR_D:	.long	0x00000004
PLLCR_A:	.long	PLLCR
PLLCR_D:	.long	0x00004000
FRQCRA_A:	.long	FRQCRA
FRQCRA_D:	.long	0x8E003508
FRQCRB_A:	.long	FRQCRB
FRQCRB_D:	.long	0x0
MMSELR_A:	.long	MMSELR
MMSELR_D:	.long	0xA5A50000
CMNCR_A:	.long	CMNCR
CMNCR_D:	.long	0x00000013
CS0BCR_A:	.long	CS0BCR
CS0BCR_D:	.long	0x11110400
CS0WCR_A:	.long	CS0WCR
CS0WCR_D:	.long	0x00000440
DBPDCNT0_A:	.long	DBPDCNT0
DBPDCNT0_D0: .long	0x00000181
DBPDCNT0_D1: .long	0x00000080
DBCONF_A:	.long	DBCONF
DBCONF_D:	.long	0x015B0002
DBTR0_A:	.long 	DBTR0
DBTR0_D:	.long 	0x03061502
DBTR1_A:	.long	DBTR1
DBTR1_D:	.long	0x02020102
DBTR2_A:	.long	DBTR2
DBTR2_D:	.long	0x01090305
DBTR3_A:	.long	DBTR3
DBTR3_D:	.long	0x00000002
DBKIND_A:	.long	DBKIND
DBKIND_D:	.long	0x00000005
DBCKECNT_A:	.long	DBCKECNT
DBCKECNT_D:	.long	0x00000001
DBCMDCNT_A:	.long	DBCMDCNT
DBCMDCNT_D0:.long	0x2
DBCMDCNT_D1:.long	0x4
DBMRCNT_A:	.long	DBMRCNT
DBMRCNT_D0:	.long	0x00020000
DBMRCNT_D1:	.long	0x00030000
DBMRCNT_D2:	.long	0x00010040
DBMRCNT_D3:	.long	0x00000532
DBMRCNT_D4:	.long	0x00000432
DBMRCNT_D5:	.long	0x000103C0
DBMRCNT_D6:	.long	0x00010040
DBEN_A:		.long	DBEN
DBEN_D:		.long	0x01
DBRFPDN0_A:	.long	DBRFPDN0
DBRFPDN1_A:	.long	DBRFPDN1
DBRFPDN2_A:	.long	DBRFPDN2
DBRFPDN0_D:	.long	0x00010000
DBRFPDN1_D:	.long	0x00000613
DBRFPDN2_D:	.long	0x238C003A
SDRAM_A:	.long	0xa8000000
DUMMY_A:	.long	0x0c400000
CCR_A:		.long	CCR
CCR_D:		.long	0x0000090B
SR_MASK_D:	.long	0xEFFFFF0F
