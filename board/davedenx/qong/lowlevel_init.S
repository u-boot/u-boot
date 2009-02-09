/*
 * Copyright (C) 2009, Emcraft Systems, Ilya Yanok <yanok@emcraft.com>
 *
 * Based on board/freescale/mx31ads/lowlevel_init.S
 * by Guennadi Liakhovetski.
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

#include <asm/arch/mx31-regs.h>

.macro REG reg, val
	ldr r2, =\reg
	ldr r3, =\val
	str r3, [r2]
.endm

.macro REG8 reg, val
	ldr r2, =\reg
	ldr r3, =\val
	strb r3, [r2]
.endm

.macro DELAY loops
	ldr r2, =\loops
1:
	subs	r2, r2, #1
	nop
	bcs 1b
.endm

/* RedBoot: To support 133MHz DDR */
.macro init_drive_strength
	/*
	 * Disable maximum drive strength SDRAM/DDR lines by clearing DSE1 bits
	 * in SW_PAD_CTL registers
	 */

	/* SDCLK */
	ldr r1, =IOMUXC_SW_PAD_CTL(0x2b)
	ldr r0, [r1, #0x6C]
	bic r0, r0, #(1 << 12)
	str r0, [r1, #0x6C]

	/* CAS */
	ldr r0, [r1, #0x70]
	bic r0, r0, #(1 << 22)
	str r0, [r1, #0x70]

	/* RAS */
	ldr r0, [r1, #0x74]
	bic r0, r0, #(1 << 2)
	str r0, [r1, #0x74]

	/* CS2 (CSD0) */
	ldr r0, [r1, #0x7C]
	bic r0, r0, #(1 << 22)
	str r0, [r1, #0x7C]

	/* DQM3 */
	ldr r0, [r1, #0x84]
	bic r0, r0, #(1 << 22)
	str r0, [r1, #0x84]

	/* DQM2, DQM1, DQM0, SD31-SD0, A25-A0, MA10 (0x288..0x2DC) */
	ldr r2, =22	/* (0x2E0 - 0x288) / 4 = 22 */
pad_loop:
	ldr r0, [r1, #0x88]
	bic r0, r0, #(1 << 22)
	bic r0, r0, #(1 << 12)
	bic r0, r0, #(1 << 2)
	str r0, [r1, #0x88]
	add r1, r1, #4
	subs r2, r2, #0x1
	bne pad_loop
.endm /* init_drive_strength */

.globl lowlevel_init
lowlevel_init:

	init_drive_strength

	/* Image Processing Unit: */
	/* Too early to switch display on? */
	/* Switch on Display Interface */
	REG	IPU_CONF, IPU_CONF_DI_EN
	/* Clock Control Module: */
	REG	CCM_CCMR, 0x074B0BF5		/* Use CKIH, MCU PLL off */

	DELAY 0x40000

	REG	CCM_CCMR, 0x074B0BF5 | CCMR_MPE			/* MCU PLL on */
	/* Switch to MCU PLL */
	REG	CCM_CCMR, (0x074B0BF5 | CCMR_MPE) & ~CCMR_MDS

	/* 399-133-66.5 */
	ldr	r0, =CCM_BASE
	ldr	r1, =0xFF871650
	/* PDR0 */
	str	r1, [r0, #0x4]
	ldr	r1, MPCTL_PARAM_399
	/* MPCTL */
	str	r1, [r0, #0x10]

	/* Set UPLL=240MHz, USB=60MHz */
	ldr	r1, =0x49FCFE7F
	/* PDR1 */
	str	r1, [r0, #0x8]
	ldr	r1, UPCTL_PARAM_240
	/* UPCTL */
	str	r1, [r0, #0x14]
	/* default CLKO to 1/8 of the ARM core */
	mov	r1, #0x00000208
	/* COSR */
	str	r1, [r0, #0x1c]

	/* Default: 1, 4, 12, 1 */
	REG	CCM_SPCTL, PLL_PD(1) | PLL_MFD(4) | PLL_MFI(12) | PLL_MFN(1)

	/* B8xxxxxx - NAND, 8xxxxxxx - CSD0 RAM */
	REG	0xB8001010, 0x00000004
	REG	0xB8001004, ((3 << 21) | /* tXP */	\
			     (0 << 20) | /* tWTR */	\
			     (2 << 18) | /* tRP */	\
			     (1 << 16) | /* tMRD */	\
			     (0 << 15) | /* tWR */	\
			     (5 << 12) | /* tRAS */	\
			     (1 << 10) | /* tRRD */	\
			     (3 << 8)  | /* tCAS */	\
			     (2 << 4)  | /* tRCD */	\
			     (7 << 0)    /* tRC */ )
	REG	0xB8001000, 0x92100000
	REG	0x80000f00, 0x12344321
	REG	0xB8001000, 0xa2100000
	REG	0x80000000, 0x12344321
	REG	0x80000000, 0x12344321
	REG	0xB8001000, 0xb2100000
	REG8	0x80000033, 0xda
	REG8	0x81000000, 0xff
	REG	0xB8001000, ((1 << 31) |				\
			     (0 << 28) |				\
			     (0 << 27) |				\
			     (3 << 24) | /* 14 rows */			\
			     (2 << 20) | /* 10 cols */			\
			     (2 << 16) |				\
			     (4 << 13) | /* 3.91us (64ms/16384) */	\
			     (0 << 10) |				\
			     (0 << 8)  |				\
			     (1 << 7)  |				\
			     (0 << 0))
	REG	0x80000000, 0xDEADBEEF
	REG	0xB8001010, 0x0000000c

	mov	pc, lr

MPCTL_PARAM_399:
	.word (((1 - 1) << 26) + ((52 - 1) << 16) + (7 << 10) + (35 << 0))
UPCTL_PARAM_240:
	.word (((2 - 1) << 26) + ((13 - 1) << 16) + (9 << 10) + (3  << 0))
