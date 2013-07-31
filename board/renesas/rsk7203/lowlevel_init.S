/*
 * Copyright (C) 2008 Nobuhiro Iwamatsu
 * Copyright (C) 2008 Renesas Solutions Corp.
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

	/* ConfigurePortPins */
	write16	PECRL3_A, PECRL3_D

	write16	PCCRL4_A, PCCRL4_D0

	write16	PECRL4_A, PECRL4_D0

	write16	PEIORL_A, PEIORL_D0

	write16	PCIORL_A, PCIORL_D

	write16	PFCRH2_A, PFCRH2_D

	write16	PFCRH3_A, PFCRH3_D

	write16	PFCRH1_A, PFCRH1_D

	write16	PFIORH_A, PFIORH_D

	write16	PECRL1_A, PECRL1_D0

	write16	PEIORL_A, PEIORL_D1

	/* Configure Operating Frequency */
	write16	WTCSR_A, WTCSR_D0

	write16	WTCSR_A, WTCSR_D1

	write16	WTCNT_A, WTCNT_D

	/* Set clock mode*/
	write16	FRQCR_A, FRQCR_D

	/* Configure Bus And Memory */
init_bsc_cs0:
	write16	PCCRL4_A, PCCRL4_D1

	write16	PECRL1_A, PECRL1_D1

	write32	CMNCR_A, CMNCR_D

	write32	CS0BCR_A, CS0BCR_D

	write32	CS0WCR_A, CS0WCR_D

init_bsc_cs1:
	write16	PECRL4_A, PECRL4_D1

	write32	CS1WCR_A, CS1WCR_D

init_sdram:
	write16	PCCRL2_A, PCCRL2_D

	write16	PCCRL4_A, PCCRL4_D2

	write16	PCCRL1_A, PCCRL1_D

	write16	PCCRL3_A, PCCRL3_D

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
PCCRL4_A:	.long 0xFFFE3910
PCCRL4_D0:	.word 0x0000
.align 2
PECRL4_A:	.long 0xFFFE3A10
PECRL4_D0:	.word 0x0000
.align 2
PECRL3_A:	.long 0xFFFE3A12
PECRL3_D:	.word 0x0000
.align 2
PEIORL_A:	.long 0xFFFE3A06
PEIORL_D0:	.word 0x1C00
PEIORL_D1:	.word 0x1C02
PCIORL_A:	.long 0xFFFE3906
PCIORL_D:	.word 0x4000
.align 2
PFCRH2_A:	.long 0xFFFE3A8C
PFCRH2_D:	.word 0x0000
.align 2
PFCRH3_A:	.long 0xFFFE3A8A
PFCRH3_D:	.word 0x0000
.align 2
PFCRH1_A:	.long 0xFFFE3A8E
PFCRH1_D:	.word 0x0000
.align 2
PFIORH_A:	.long 0xFFFE3A84
PFIORH_D:	.word 0x0729
.align 2
PECRL1_A:	.long 0xFFFE3A16
PECRL1_D0:	.word 0x0033
.align 2


WTCSR_A:	.long 0xFFFE0000
WTCSR_D0:	.word 0xA518
WTCSR_D1:	.word 0xA51D
WTCNT_A:	.long 0xFFFE0002
WTCNT_D:	.word 0x5A84
.align 2
FRQCR_A:	.long 0xFFFE0010
FRQCR_D:	.word 0x0104
.align 2

PCCRL4_D1:	.word 0x0010
PECRL1_D1:	.word 0x0133

CMNCR_A:	.long 0xFFFC0000
CMNCR_D:	.long 0x00001810
CS0BCR_A:	.long 0xFFFC0004
CS0BCR_D:	.long 0x10000400
CS0WCR_A:	.long 0xFFFC0028
CS0WCR_D:	.long 0x00000B41
PECRL4_D1:	.word 0x0100
.align 2
CS1WCR_A:	.long 0xFFFC002C
CS1WCR_D:	.long 0x00000B01
PCCRL4_D2:	.word 0x0011
.align 2
PCCRL3_A:	.long 0xFFFE3912
PCCRL3_D:	.word 0x0011
.align 2
PCCRL2_A:	.long 0xFFFE3914
PCCRL2_D:	.word 0x1111
.align 2
PCCRL1_A:	.long 0xFFFE3916
PCCRL1_D:	.word 0x1010
.align 2
PDCRL4_A:	.long 0xFFFE3990
PDCRL4_D:	.word 0x0011
.align 2
PDCRL3_A:	.long 0xFFFE3992
PDCRL3_D:	.word 0x00011
.align 2
PDCRL2_A:	.long 0xFFFE3994
PDCRL2_D:	.word 0x1111
.align 2
PDCRL1_A:	.long 0xFFFE3996
PDCRL1_D:	.word 0x1000
.align 2
CS3BCR_A:	.long 0xFFFC0010
CS3BCR_D:	.long 0x00004400
CS3WCR_A:	.long 0xFFFC0034
CS3WCR_D:	.long 0x00002892
SDCR_A:		.long 0xFFFC004C
SDCR_D:		.long 0x00000809
RTCOR_A:	.long 0xFFFC0058
RTCOR_D:	.long 0xA55A0041
RTCSR_A:	.long 0xFFFC0050
RTCSR_D:	.long 0xa55a0010

SDRAM_MODE:	.long 0xFFFC5040
REPEAT_D:	.long 0x00009C40
