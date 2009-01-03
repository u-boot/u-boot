/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl) and
 *	          Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 * Modified for the at91rm9200dk board by
 * (C) Copyright 2004
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <version.h>

#ifdef CONFIG_BOOTBINFUNC
/*
 * some parameters for the board
 *
 * This is based on rm9200dk.cfg for the BDI2000 from ABATRON which in
 * turn is based on the boot.bin code from ATMEL
 *
 */

/* flash */
#define MC_PUIA 0xFFFFFF10
#define CONFIG_SYS_MC_PUIA_VAL 0x00000000
#define MC_PUP 0xFFFFFF50
#define CONFIG_SYS_MC_PUP_VAL 0x00000000
#define MC_PUER 0xFFFFFF54
#define CONFIG_SYS_MC_PUER_VAL 0x00000000
#define MC_ASR 0xFFFFFF04
#define CONFIG_SYS_MC_ASR_VAL 0x00000000
#define MC_AASR 0xFFFFFF08
#define CONFIG_SYS_MC_AASR_VAL 0x00000000
#define EBI_CFGR 0xFFFFFF64
#define CONFIG_SYS_EBI_CFGR_VAL 0x00000000
#define SMC_CSR0 0xFFFFFF70
#define CONFIG_SYS_SMC_CSR0_VAL 0x00003284 /* 16bit, 2 TDF, 4 WS */

/* clocks */
#define PLLAR 0xFFFFFC28
#define CONFIG_SYS_PLLAR_VAL 0x20263E04 /* 179.712000 MHz for PCK */
#define PLLBR 0xFFFFFC2C
#define CONFIG_SYS_PLLBR_VAL 0x10483E0E /* 48.054857 MHz (divider by 2 for USB) */
#define MCKR 0xFFFFFC30
/* PCK/3 = MCK Master Clock = 59.904000MHz from PLLA */
#define CONFIG_SYS_MCKR_VAL	0x00000202

/* sdram */
#define PIOC_ASR 0xFFFFF870
#define CONFIG_SYS_PIOC_ASR_VAL 0xFFFF0000 /* Configure PIOC as Perip (D16/D31) */
#define PIOC_BSR 0xFFFFF874
#define CONFIG_SYS_PIOC_BSR_VAL 0x00000000
#define PIOC_PDR 0xFFFFF804
#define CONFIG_SYS_PIOC_PDR_VAL 0xFFFF0000
#define EBI_CSA 0xFFFFFF60
#define CONFIG_SYS_EBI_CSA_VAL 0x00000002 /* CS1=CONFIG_SYS_SDRAM */
#define SDRC_CR 0xFFFFFF98
#define CONFIG_SYS_SDRC_CR_VAL 0x2188c155 /* set up the CONFIG_SYS_SDRAM */
#define CONFIG_SYS_SDRAM 0x20000000 /* address of the CONFIG_SYS_SDRAM */
#define CONFIG_SYS_SDRAM1 0x20000080 /* address of the CONFIG_SYS_SDRAM */
#define CONFIG_SYS_SDRAM_VAL 0x00000000 /* value written to CONFIG_SYS_SDRAM */
#define SDRC_MR 0xFFFFFF90
#define CONFIG_SYS_SDRC_MR_VAL 0x00000002 /* Precharge All */
#define CONFIG_SYS_SDRC_MR_VAL1 0x00000004 /* refresh */
#define CONFIG_SYS_SDRC_MR_VAL2 0x00000003 /* Load Mode Register */
#define CONFIG_SYS_SDRC_MR_VAL3 0x00000000 /* Normal Mode */
#define SDRC_TR 0xFFFFFF94
#define CONFIG_SYS_SDRC_TR_VAL 0x000002E0 /* Write refresh rate */

_TEXT_BASE:
	.word   TEXT_BASE

.globl lowlevelinit
lowlevelinit:
	/* memory control configuration */
	/* this isn't very elegant, but  what the heck */
	ldr     r0, =SMRDATA
	ldr     r1, _TEXT_BASE
	sub     r0, r0, r1
	add     r2, r0, #80
0:
	/* the address */
	ldr     r1, [r0], #4
	/* the value */
	ldr     r3, [r0], #4
	str     r3, [r1]
	cmp     r2, r0
	bne     0b
	/* delay - this is all done by guess */
	ldr     r0, =0x00010000
1:
	subs    r0, r0, #1
	bhi     1b
	ldr     r0, =SMRDATA1
	ldr     r1, _TEXT_BASE
	sub     r0, r0, r1
	add     r2, r0, #176
2:
	/* the address */
	ldr     r1, [r0], #4
	/* the value */
	ldr     r3, [r0], #4
	str     r3, [r1]
	cmp     r2, r0
	bne     2b

	/* everything is fine now */
	mov     pc, lr

	.ltorg

SMRDATA:
	.word MC_PUIA
	.word CONFIG_SYS_MC_PUIA_VAL
	.word MC_PUP
	.word CONFIG_SYS_MC_PUP_VAL
	.word MC_PUER
	.word CONFIG_SYS_MC_PUER_VAL
	.word MC_ASR
	.word CONFIG_SYS_MC_ASR_VAL
	.word MC_AASR
	.word CONFIG_SYS_MC_AASR_VAL
	.word EBI_CFGR
	.word CONFIG_SYS_EBI_CFGR_VAL
	.word SMC_CSR0
	.word CONFIG_SYS_SMC_CSR0_VAL
	.word PLLAR
	.word CONFIG_SYS_PLLAR_VAL
	.word PLLBR
	.word CONFIG_SYS_PLLBR_VAL
	.word MCKR
	.word CONFIG_SYS_MCKR_VAL
	/* SMRDATA is 80 bytes long */
	/* here there's a delay of 100 */
SMRDATA1:
	.word PIOC_ASR
	.word CONFIG_SYS_PIOC_ASR_VAL
	.word PIOC_BSR
	.word CONFIG_SYS_PIOC_BSR_VAL
	.word PIOC_PDR
	.word CONFIG_SYS_PIOC_PDR_VAL
	.word EBI_CSA
	.word CONFIG_SYS_EBI_CSA_VAL
	.word SDRC_CR
	.word CONFIG_SYS_SDRC_CR_VAL
	.word SDRC_MR
	.word CONFIG_SYS_SDRC_MR_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word SDRC_MR
	.word CONFIG_SYS_SDRC_MR_VAL1
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word SDRC_MR
	.word CONFIG_SYS_SDRC_MR_VAL2
	.word CONFIG_SYS_SDRAM1
	.word CONFIG_SYS_SDRAM_VAL
	.word SDRC_TR
	.word CONFIG_SYS_SDRC_TR_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word SDRC_MR
	.word CONFIG_SYS_SDRC_MR_VAL3
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	/* SMRDATA1 is 176 bytes long */
#endif /* CONFIG_BOOTBINFUNC */
