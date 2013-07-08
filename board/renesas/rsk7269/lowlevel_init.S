/*
 * Copyright (C) 2012 Renesas Electronics Europe Ltd.
 * Copyright (C) 2012 Phil Edworthy
 * Copyright (C) 2008 Renesas Solutions Corp.
 * Copyright (C) 2008 Nobuhiro Iwamatsu
 *
 * Based on board/renesas/rsk7264/lowlevel_init.S
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
	/* Flush and enable caches (data cache in write-through mode) */
	write32	CCR1_A ,CCR1_D

	/* Disable WDT */
	write16	WTCSR_A, WTCSR_D
	write16	WTCNT_A, WTCNT_D

	/* Disable Register Bank interrupts */
	write16 IBNR_A, IBNR_D

	/* Set clocks based on 13.225MHz xtal */
	write16	FRQCR_A, FRQCR_D	/* CPU=266MHz, I=133MHz, P=66MHz */

	/* Enable all peripherals */
	write8 STBCR3_A, STBCR3_D
	write8 STBCR4_A, STBCR4_D
	write8 STBCR5_A, STBCR5_D
	write8 STBCR6_A, STBCR6_D
	write8 STBCR7_A, STBCR7_D
	write8 STBCR8_A, STBCR8_D
	write8 STBCR9_A, STBCR9_D
	write8 STBCR10_A, STBCR10_D

	/* SCIF7 and IIC2 */
	write16 PJCR3_A, PJCR3_D	/* TXD7 */
	write16 PECR1_A, PECR1_D	/* RXD7, SDA2, SCL2 */

	/* Configure bus (CS0) */
	write16 PFCR3_A, PFCR3_D	/* A24 */
	write16 PFCR2_A, PFCR2_D	/* A23 and CS1# */
	write16 PBCR5_A, PBCR5_D	/* A22, A21, A20 */
	write16 PCCR0_A, PCCR0_D	/* DQMLL#, RD/WR# */
	write32 CS0WCR_A, CS0WCR_D
	write32 CS0BCR_A, CS0BCR_D

	/* Configure SDRAM (CS3) */
	write16 PCCR2_A, PCCR2_D	/* CS3# */
	write16 PCCR1_A, PCCR1_D	/* CKE, CAS#, RAS#, DQMLU# */
	write16 PCCR0_A, PCCR0_D	/* DQMLL#, RD/WR# */
	write32	CS3BCR_A, CS3BCR_D
	write32	CS3WCR_A, CS3WCR_D
	write32	SDCR_A, SDCR_D
	write32	RTCOR_A, RTCOR_D
	write32	RTCSR_A, RTCSR_D

	/* Configure ethernet (CS1) */
	write16 PHCR1_A, PHCR1_D	/* PINT5 on PH5 */
	write16 PHCR0_A, PHCR0_D
	write16 PFCR2_A, PFCR2_D	/* CS1# */
	write32	CS1BCR_A, CS1BCR_D	/* Big endian */
	write32	CS1WCR_A, CS1WCR_D	/* 1 cycle */
	write16 PJDR1_A, PJDR1_D	/* FIFO-SEL = 1 */
	write16 PJIOR1_A, PJIOR1_D

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

STBCR3_A:	.long 0xFFFE0408
STBCR4_A:	.long 0xFFFE040C
STBCR5_A:	.long 0xFFFE0410
STBCR6_A:	.long 0xFFFE0414
STBCR7_A:	.long 0xFFFE0418
STBCR8_A:	.long 0xFFFE041C
STBCR9_A:	.long 0xFFFE0440
STBCR10_A:	.long 0xFFFE0444
STBCR3_D:	.long 0x0000001A
STBCR4_D:	.long 0x00000000
STBCR5_D:	.long 0x00000000
STBCR6_D:	.long 0x00000000
STBCR7_D:	.long 0x00000012
STBCR8_D:	.long 0x00000009
STBCR9_D:	.long 0x00000000
STBCR10_D:	.long 0x00000010

WTCSR_A:	.long 0xFFFE0000
WTCNT_A:	.long 0xFFFE0002
WTCSR_D:	.word 0xA518
WTCNT_D:	.word 0x5A00

IBNR_A:		.long 0xFFFE080E
IBNR_D:		.word 0x0000
.align 2
FRQCR_A:	.long 0xFFFE0010
FRQCR_D:	.word 0x0015
.align 2

PJCR3_A:	.long 0xFFFE3908
PJCR3_D:	.word 0x5000
.align 2
PECR1_A:	.long 0xFFFE388C
PECR1_D:	.word 0x2011
.align 2

PFCR3_A:	.long 0xFFFE38A8
PFCR2_A:	.long 0xFFFE38AA
PBCR5_A:	.long 0xFFFE3824
PFCR3_D:	.word 0x0010
PFCR2_D:	.word 0x0101
PBCR5_D:	.word 0x0111
.align 2
CS0WCR_A:	.long 0xFFFC0028
CS0WCR_D:	.long 0x00000341
CS0BCR_A:	.long 0xFFFC0004
CS0BCR_D:	.long 0x00000400

PCCR2_A:	.long 0xFFFE384A
PCCR1_A:	.long 0xFFFE384C
PCCR0_A:	.long 0xFFFE384E
PCCR2_D:	.word 0x0001
PCCR1_D:	.word 0x1111
PCCR0_D:	.word 0x1111
.align 2
CS3BCR_A:	.long 0xFFFC0010
CS3BCR_D:	.long 0x00004400
CS3WCR_A:	.long 0xFFFC0034
CS3WCR_D:	.long 0x00004912
SDCR_A:		.long 0xFFFC004C
SDCR_D:		.long 0x00000811
RTCOR_A:	.long 0xFFFC0058
RTCOR_D:	.long 0xA55A0035
RTCSR_A:	.long 0xFFFC0050
RTCSR_D:	.long 0xA55A0010
.align 2
SDRAM_MODE:	.long 0xFFFC5460
REPEAT_D:	.long 0x000033F1

PHCR1_A:	.long 0xFFFE38EC
PHCR0_A:	.long 0xFFFE38EE
PHCR1_D:	.word 0x2222
PHCR0_D:	.word 0x2222
.align 2
CS1BCR_A:	.long 0xFFFC0008
CS1BCR_D:	.long 0x00000400
CS1WCR_A:	.long 0xFFFC002C
CS1WCR_D:	.long 0x00000080
PJDR1_A:	.long 0xFFFE3914
PJDR1_D:	.word 0x0000
.align 2
PJIOR1_A:	.long 0xFFFE3910
PJIOR1_D:	.word 0x8000
.align 2
