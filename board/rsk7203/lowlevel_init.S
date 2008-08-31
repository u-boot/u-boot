/*
 * Copyright (C) 2008 Nobuhiro Iwamatsu
 * Copyright (C) 2008 Renesas Solutions Corp.
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
	/* Cache setting */
	mov.l CCR1_A ,r1
	mov.l CCR1_D ,r0
	mov.l r0,@r1

	/* ConfigurePortPins */
	mov.l PECRL3_A, r1
	mov.l PECRL3_D, r0
	mov.w r0,@r1

	mov.l PCCRL4_A, r1
	mov.l PCCRL4_D0, r0
	mov.w r0,@r1

	mov.l PECRL4_A, r1
	mov.l PECRL4_D0, r0
	mov.w r0,@r1

	mov.l PEIORL_A, r1
	mov.l PEIORL_D0, r0
	mov.w r0,@r1

	mov.l PCIORL_A, r1
	mov.l PCIORL_D, r0
	mov.w r0,@r1

	mov.l PFCRH2_A, r1
	mov.l PFCRH2_D, r0
	mov.w r0,@r1

	mov.l PFCRH3_A, r1
	mov.l PFCRH3_D, r0
	mov.w r0,@r1

	mov.l PFCRH1_A, r1
	mov.l PFCRH1_D, r0
	mov.w r0,@r1

	mov.l PFIORH_A, r1
	mov.l PFIORH_D, r0
	mov.w r0,@r1

	mov.l PECRL1_A, r1
	mov.l PECRL1_D0, r0
	mov.w r0,@r1

	mov.l PEIORL_A, r1
	mov.l PEIORL_D1, r0
	mov.w r0,@r1

	/* Configure Operating Frequency */
	mov.l WTCSR_A ,r1
	mov.l WTCSR_D0 ,r0
	mov.w r0,@r1

	mov.l WTCSR_A ,r1
	mov.l WTCSR_D1 ,r0
	mov.w r0,@r1

	mov.l WTCNT_A ,r1
	mov.l WTCNT_D ,r0
	mov.w r0,@r1

	/* Set clock mode*/
	mov.l FRQCR_A,r1
	mov.l FRQCR_D,r0
	mov.w r0,@r1

	/* Configure Bus And Memory */
init_bsc_cs0:
	mov.l   PCCRL4_A,r1
	mov.l   PCCRL4_D1,r0
	mov.w   r0,@r1

	mov.l   PECRL1_A,r1
	mov.l   PECRL1_D1,r0
	mov.w   r0,@r1

	mov.l CMNCR_A,r1
	mov.l CMNCR_D,r0
	mov.l r0,@r1

	mov.l SC0BCR_A,r1
	mov.l SC0BCR_D,r0
	mov.l r0,@r1

	mov.l CS0WCR_A,r1
	mov.l CS0WCR_D,r0
	mov.l r0,@r1

init_bsc_cs1:
	mov.l   PECRL4_A,r1
	mov.l   PECRL4_D1,r0
	mov.w   r0,@r1

	mov.l CS1WCR_A,r1
	mov.l CS1WCR_D,r0
	mov.l r0,@r1

init_sdram:
	mov.l	PCCRL2_A,r1
	mov.l	PCCRL2_D,r0
	mov.w	r0,@r1

	mov.l	PCCRL4_A,r1
	mov.l	PCCRL4_D2,r0
	mov.w   r0,@r1

	mov.l   PCCRL1_A,r1
	mov.l	PCCRL1_D,r0
	mov.w   r0,@r1

	mov.l   PCCRL3_A,r1
	mov.l	PCCRL3_D,r0
	mov.w   r0,@r1

	mov.l CS3BCR_A,r1
	mov.l CS3BCR_D,r0
	mov.l r0,@r1

	mov.l CS3WCR_A,r1
	mov.l CS3WCR_D,r0
	mov.l r0,@r1

	mov.l SDCR_A,r1
	mov.l SDCR_D,r0
	mov.l r0,@r1

	mov.l RTCOR_A,r1
	mov.l RTCOR_D,r0
	mov.l r0,@r1

	mov.l RTCSR_A,r1
	mov.l RTCSR_D,r0
	mov.l r0,@r1

	/* wait 200us */
	mov.l   REPEAT_D,r3
	mov     #0,r2
repeat0:
	add     #1,r2
	cmp/hs  r3,r2
	bf      repeat0
	nop

	mov.l SDRAM_MODE, r1
	mov   #0,r0
	mov.l r0, @r1

	nop
	rts

	.align 4

CCR1_A:		.long CCR1
CCR1_D:		.long 0x0000090B
PCCRL4_A:	.long 0xFFFE3910
PCCRL4_D0:	.long 0x00000000
PECRL4_A:	.long 0xFFFE3A10
PECRL4_D0:	.long 0x00000000
PECRL3_A:	.long 0xFFFE3A12
PECRL3_D:	.long 0x00000000
PEIORL_A:	.long 0xFFFE3A06
PEIORL_D0:	.long 0x00001C00
PEIORL_D1:	.long 0x00001C02
PCIORL_A:	.long 0xFFFE3906
PCIORL_D:	.long 0x00004000
PFCRH2_A:	.long 0xFFFE3A8C
PFCRH2_D:	.long 0x00000000
PFCRH3_A:	.long 0xFFFE3A8A
PFCRH3_D:	.long 0x00000000
PFCRH1_A:	.long 0xFFFE3A8E
PFCRH1_D:	.long 0x00000000
PFIORH_A:	.long 0xFFFE3A84
PFIORH_D:	.long 0x00000729
PECRL1_A:	.long 0xFFFE3A16
PECRL1_D0:	.long 0x00000033


WTCSR_A:	.long 0xFFFE0000
WTCSR_D0: 	.long 0x0000A518
WTCSR_D1: 	.long 0x0000A51D
WTCNT_A:	.long 0xFFFE0002
WTCNT_D:	.long 0x00005A84
FRQCR_A:	.long 0xFFFE0010
FRQCR_D:	.long 0x00000104

PCCRL4_D1:	.long 0x00000010
PECRL1_D1:	.long 0x00000133

CMNCR_A:	.long 0xFFFC0000
CMNCR_D:	.long 0x00001810
SC0BCR_A:	.long 0xFFFC0004
SC0BCR_D:	.long 0x10000400
CS0WCR_A:	.long 0xFFFC0028
CS0WCR_D:	.long 0x00000B41
PECRL4_D1:	.long 0x00000100
CS1WCR_A:	.long 0xFFFC002C
CS1WCR_D:	.long 0x00000B01
PCCRL4_D2:	.long 0x00000011
PCCRL3_A:	.long 0xFFFE3912
PCCRL3_D:	.long 0x00000011
PCCRL2_A:	.long 0xFFFE3914
PCCRL2_D:	.long 0x00001111
PCCRL1_A:	.long 0xFFFE3916
PCCRL1_D:	.long 0x00001010
PDCRL4_A:	.long 0xFFFE3990
PDCRL4_D:	.long 0x00000011
PDCRL3_A:	.long 0xFFFE3992
PDCRL3_D:	.long 0x00000011
PDCRL2_A:	.long 0xFFFE3994
PDCRL2_D:	.long 0x00001111
PDCRL1_A:	.long 0xFFFE3996
PDCRL1_D:	.long 0x00001000
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

STBCR3_A:	.long 0xFFFE0408
STBCR3_D:	.long 0x00000000
STBCR4_A:	.long 0xFFFE040C
STBCR4_D:	.long 0x00000008
STBCR5_A:	.long 0xFFFE0410
STBCR5_D:	.long 0x00000000
STBCR6_A: 	.long 0xFFFE0414
STBCR6_D:	.long 0x00000002
SDRAM_MODE:	.long 0xFFFC5040
REPEAT_D:	.long 0x00009C40
