/*
 * Copyright (C) 2011 Renesas Electronics Europe Ltd.
 * Copyright (C) 2008 Renesas Solutions Corp.
 * Copyright (C) 2008 Nobuhiro Iwamatsu
 *
 * Based on board/renesas/rsk7203/lowlevel_init.S
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <config.h>
#include <version.h>

#include <asm/processor.h>
#include <asm/macro.h>

	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:
	/* Cache setting */
	write32	CCR1_A ,CCR1_D

	/* io_set_cpg */
	write8 STBCR3_A, STBCR3_D
	write8 STBCR4_A, STBCR4_D
	write8 STBCR5_A, STBCR5_D
	write8 STBCR6_A, STBCR6_D
	write8 STBCR7_A, STBCR7_D
	write8 STBCR8_A, STBCR8_D

	/* ConfigurePortPins */

	/* Leaving LED1 ON for sanity test */
	write16 PJCR1_A, PJCR1_D1
	write16 PJCR2_A, PJCR2_D
	write16 PJIOR0_A, PJIOR0_D1
	write16 PJDR0_A, PJDR0_D
	write16 PJPR0_A, PJPR0_D

	/* Configure EN_PIN & RS_PIN */
	write16 PGCR2_A, PGCR2_D
	write16 PGIOR0_A, PGIOR0_D

	/* Configure the port pins connected to UART */
	write16 PJCR1_A, PJCR1_D2
	write16 PJIOR0_A, PJIOR0_D2

	/* Configure Operating Frequency */
	write16	WTCSR_A, WTCSR_D0
	write16	WTCSR_A, WTCSR_D1
	write16	WTCNT_A, WTCNT_D

	/* Control of RESBANK */
	write16 IBNR_A, IBNR_D
	/* Enable SCIF3 module */
	write16 STBCR4_A, STBCR4_D

	/* Set clock mode*/
	write16	FRQCR_A, FRQCR_D

	/* Configure Bus And Memory */
init_bsc_cs0:

pfc_settings:
	write16 PCCR2_A, PCCR2_D
	write16 PCCR1_A, PCCR1_D
	write16 PCCR0_A, PCCR0_D

	write16 PBCR0_A, PBCR0_D
	write16 PBCR1_A, PBCR1_D
	write16 PBCR2_A, PBCR2_D
	write16 PBCR3_A, PBCR3_D
	write16 PBCR4_A, PBCR4_D
	write16 PBCR5_A, PBCR5_D

	write16 PDCR0_A, PDCR0_D
	write16 PDCR1_A, PDCR1_D
	write16 PDCR2_A, PDCR2_D
	write16 PDCR3_A, PDCR3_D

	write32 CS0WCR_A, CS0WCR_D
	write32 CS0BCR_A, CS0BCR_D

init_bsc_cs2:
	write16	PJCR0_A, PJCR0_D
	write32	CS2WCR_A, CS2WCR_D

init_sdram:
	write32	CS3BCR_A, CS3BCR_D
	write32	CS3WCR_A, CS3WCR_D
	write32	SDCR_A, SDCR_D
	write32	RTCOR_A, RTCOR_D
	write32	RTCSR_A, RTCSR_D

	/* wait 200us */
	mov.l	REPEAT_D, r3
	mov	#0, r2
repeat0:
	add	#1, r2
	cmp/hs	r3, r2
	bf	repeat0
	nop

	mov.l	SDRAM_MODE, r1
	mov	#0, r0
	mov.l	r0, @r1

	nop
	rts

	.align 4

CCR1_A:		.long CCR1
CCR1_D:		.long 0x0000090B
FRQCR_A:	.long 0xFFFE0010
FRQCR_D:	.word 0x1003
.align 2
STBCR3_A:	.long 0xFFFE0408
STBCR3_D:	.long 0x00000002
STBCR4_A:	.long 0xFFFE040C
STBCR4_D:	.word 0x0000
.align 2
STBCR5_A:	.long 0xFFFE0410
STBCR5_D:	.long 0x00000010
STBCR6_A:	.long 0xFFFE0414
STBCR6_D:	.long 0x00000002
STBCR7_A:	.long 0xFFFE0418
STBCR7_D:	.long 0x0000002A
STBCR8_A:	.long 0xFFFE041C
STBCR8_D:	.long 0x0000007E
PJCR1_A:	.long 0xFFFE390C
PJCR1_D1:	.word 0x0000
PJCR1_D2:	.word 0x0022
PJCR2_A:	.long 0xFFFE390A
PJCR2_D:	.word 0x0000
.align 2
PJIOR0_A:	.long 0xFFFE3912
PJIOR0_D1:	.word 0x0FC0
PJIOR0_D2:	.word 0x0FE0
PJDR0_A:	.long 0xFFFE3916
PJDR0_D:	.word 0x0FBF
.align 2
PJPR0_A:	.long 0xFFFE391A
PJPR0_D:	.long 0x00000FBF
PGCR2_A:	.long 0xFFFE38CA
PGCR2_D:	.word 0x0000
.align 2
PGIOR0_A:	.long 0xFFFE38D2
PGIOR0_D:	.word 0x03F0
.align 2
WTCSR_A:	.long 0xFFFE0000
WTCSR_D0:	.word 0x0000
WTCSR_D1:	.word 0x0000
WTCNT_A:	.long 0xFFFE0002
WTCNT_D:	.word 0x0000
.align 2
PCCR0_A:	.long 0xFFFE384E
PDCR0_A:	.long 0xFFFE386E
PDCR1_A:	.long 0xFFFE386C
PDCR2_A:	.long 0xFFFE386A
PDCR3_A:	.long 0xFFFE3868
PBCR0_A:	.long 0xFFFE382E
PBCR1_A:	.long 0xFFFE382C
PBCR2_A:	.long 0xFFFE382A
PBCR3_A:	.long 0xFFFE3828
PBCR4_A:	.long 0xFFFE3826
PBCR5_A:	.long 0xFFFE3824
PCCR0_D:	.word 0x1111
PDCR0_D:	.word 0x1111
PDCR1_D:	.word 0x1111
PDCR2_D:	.word 0x1111
PDCR3_D:	.word 0x1111
PBCR0_D:	.word 0x1110
PBCR1_D:	.word 0x1111
PBCR2_D:	.word 0x1111
PBCR3_D:	.word 0x1111
PBCR4_D:	.word 0x1111
PBCR5_D:	.word 0x0111
.align 2
CS0WCR_A:	.long 0xFFFC0028
CS0WCR_D:	.long 0x00000B41
CS0BCR_A:	.long 0xFFFC0004
CS0BCR_D:	.long 0x10000400
PJCR0_A:	.long 0xFFFE390E
PJCR0_D:	.word 0x3300
.align 2
CS2WCR_A:	.long 0xFFFC0030
CS2WCR_D:	.long 0x00000B01
PCCR2_A:	.long 0xFFFE384A
PCCR2_D:	.word 0x0001
.align 2
PCCR1_A:	.long 0xFFFE384C
PCCR1_D:	.word 0x1111
.align 2
CS3BCR_A:	.long 0xFFFC0010
CS3BCR_D:	.long 0x00004400
CS3WCR_A:	.long 0xFFFC0034
CS3WCR_D:	.long 0x0000288A
SDCR_A:		.long 0xFFFC004C
SDCR_D:		.long 0x00000812
RTCOR_A:	.long 0xFFFC0058
RTCOR_D:	.long 0xA55A0046
RTCSR_A:	.long 0xFFFC0050
RTCSR_D:	.long 0xA55A0010
IBNR_A:		.long 0xFFFE080E
IBNR_D:	.word 0x0000
.align 2
SDRAM_MODE:	.long 0xFFFC5040
REPEAT_D:	.long 0x00000085
