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

.macro SETUP_RAM cfg, ctl
	/* B8xxxxxx - NAND, 8xxxxxxx - CSD0 RAM */
	REG	0xB8001010, 0x00000004
	ldr r3, =\cfg
	ldr r2, =WEIM_ESDCFG0
	str r3, [r2]
	REG	0xB8001000, 0x92100000
	REG	0x80000f00, 0x12344321
	REG	0xB8001000, 0xa2100000
	REG	0x80000000, 0x12344321
	REG	0x80000000, 0x12344321
	REG	0xB8001000, 0xb2100000
	REG8	0x80000033, 0xda
	REG8	0x81000000, 0xff
	ldr r3, =\ctl
	ldr r2, =WEIM_ESDCTL0
	str r3, [r2]
	REG	0x80000000, 0xDEADBEEF
	REG	0xB8001010, 0x0000000c

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

check_ddr_module:
/* Set stackpointer in internal RAM to call get_ram_size */
	ldr	sp, =(IRAM_BASE_ADDR + IRAM_SIZE - 16)
	stmfd	sp!, {r0-r11, ip, lr}
	mov	ip, lr		/* save link reg across call */

	ldr	r0,=0x08000000
	SETUP_RAM	ESDCFG0_256MB, ESDCTL0_256MB
	ldr	r0,=0x80000000
	ldr	r1,=0x10000000
	bl	get_ram_size
	ldr	r1,=0x10000000
	cmp	r0,r1
	beq	restore_regs
	SETUP_RAM	ESDCFG0_128MB, ESDCTL0_128MB
	ldr	r0,=0x80000000
	ldr	r1,=0x08000000
	bl	get_ram_size
	ldr	r1,=0x08000000
	cmp	r0,r1
	beq	restore_regs

restore_regs:
	ldmfd	sp!, {r0-r11, ip, lr}
	mov	lr, ip		/* restore link reg */

	mov	pc, lr


MPCTL_PARAM_399:
	.word (((1 - 1) << 26) + ((52 - 1) << 16) + (7 << 10) + (35 << 0))
UPCTL_PARAM_240:
	.word (((2 - 1) << 26) + ((13 - 1) << 16) + (9 << 10) + (3  << 0))

	.equ	ESDCFG0_128MB, \
		(0 << 21) + /* tXP */ \
		(1 << 20) + /* tWTR */ \
		(2 << 18) + /* tRP */ \
		(1 << 16) + /* tMRD */ \
		(0 << 15) + /* tWR */ \
		(5 << 12) + /* tRAS */ \
		(1 << 10) + /* tRRD */ \
		(3 << 8) + /* tCAS */ \
		(2 << 4) + /* tRCD */ \
		(0x0F << 0) /* tRC */

	.equ	ESDCTL0_128MB, \
		(1 << 31)  +	/* enable */ \
		(0 << 28)  +	/* mode */ \
		(0 << 27)  +	/* supervisor protect */ \
		(2 << 24)  +	/* 13 rows */ \
		(2 << 20)  +	/* 10 cols */ \
		(2 << 16)  +	/* 32 bit */ \
		(3 << 13)  +	/* 7.81us (64ms/8192) */ \
		(0 << 10)  +	/* power down timer */ \
		(0 << 8)  +	/* full page */ \
		(1 << 7)  +	/* burst length */ \
		(0 << 0)	/* precharge timer */

	.equ	ESDCFG0_256MB, \
		(3 << 21)  + 	/* tXP */ \
		(0 << 20)  + 	/* tWTR */ \
		(2 << 18)  + 	/* tRP */ \
		(1 << 16)  + 	/* tMRD */ \
		(0 << 15)  + 	/* tWR */ \
		(5 << 12)  + 	/* tRAS */ \
		(1 << 10)  + 	/* tRRD */ \
		(3 << 8)   + 	/* tCAS */ \
		(2 << 4)   +	/* tRCD */ \
		(7 << 0)	/* tRC */

	.equ	ESDCTL0_256MB, \
		(1 << 31)  + \
		(0 << 28)  + \
		(0 << 27)  + \
		(3 << 24)  + /* 14 rows */ \
		(2 << 20)  + /* 10 cols */ \
		(2 << 16)  + \
		(4 << 13)  + /* 3.91us (64ms/16384) */ \
		(0 << 10)  + \
		(0 << 8)   + \
		(1 << 7)   + \
		(0 << 0)
