/*
 * Board specific setup info
 *
 * (C) Copyright 2004 Ales Jindra <jindra@2n.cz>
 * (C) Copyright 2005 Ladislav Michl <michl@2n.cz>
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

_TEXT_BASE:
	.word	TEXT_BASE	/* SDRAM load addr from config.mk */

OMAP5910_LPG1_BASE:		.word 0xfffbd000
OMAP5910_TIPB_SWITCHES_BASE:	.word 0xfffbc800
OMAP5910_MPU_TC_BASE:		.word 0xfffecc00
OMAP5910_MPU_CLKM_BASE:		.word 0xfffece00
OMAP5910_ULPD_PWR_MNG_BASE:	.word 0xfffe0800
OMAP5910_DPLL1_BASE:		.word 0xfffecf00
OMAP5910_GPIO_BASE:		.word 0xfffce000
OMAP5910_MPU_WD_TIMER_BASE:	.word 0xfffec800
OMAP5910_MPUI_BASE:		.word 0xfffec900

_OMAP5910_ARM_CKCTL:		.word OMAP5910_ARM_CKCTL
_OMAP5910_ARM_EN_CLK:		.word OMAP5910_ARM_EN_CLK

OMAP5910_MPUI_CTRL:		.word 0x0000ff1b

VAL_EMIFS_CS0_CONFIG:		.word 0x00009090
VAL_EMIFS_CS1_CONFIG:		.word 0x00003031
VAL_EMIFS_CS2_CONFIG:		.word 0x0000a0a1
VAL_EMIFS_CS3_CONFIG:		.word 0x0000c0c0
VAL_EMIFS_DYN_WAIT:		.word 0x00000000
/* autorefresh counter 0x246 ((64000000/13.4)-400)/8192) */
				/*     SLRF       SD_RET     ARE        SDRAM_TYPE   ARCV           SDRAM_FREQUENCY PWD     CLK */

#if (PHYS_SDRAM_1_SIZE == SZ_32M)
VAL_EMIFF_SDRAM_CONFIG:		.word ((0 << 0) | (0 << 1) | (3 << 2) | (0xf << 4) | (0x246 << 8) | (0 << 24) | (0 << 26) | (0 << 27))
#else
VAL_EMIFF_SDRAM_CONFIG:		.word ((0 << 0) | (0 << 1) | (3 << 2) | (0xd << 4) | (0x246 << 8) | (0 << 24) | (0 << 26) | (0 << 27))
#endif

VAL_EMIFF_SDRAM_CONFIG2:	.word 0x00000003
VAL_EMIFF_MRS:			.word 0x00000037

/*
 * GPIO04 - Green LED (Red LED is connected to LED Pulse Generator)
 * GPIO07 - LAN91C111 reset
 */
GPIO_DIRECTION:
	.word 0x0000ff6f
/*
 * Disable everything (green LED is connected via invertor)
 */
GPIO_OUTPUT:
	.word 0x00000010

MUX_CONFIG_BASE:
	.word 0xfffe1000

MUX_CONFIG_VALUES:
	.align 4
	.word 0x00000000	@ FUNC_MUX_CTRL_0
	.word 0x00000000	@ FUNC_MUX_CTRL_1
	.word 0x00000000	@ FUNC_MUX_CTRL_2
	.word 0x00000000	@ FUNC_MUX_CTRL_3
	.word 0x00000000	@ FUNC_MUX_CTRL_4
	.word 0x02080480	@ FUNC_MUX_CTRL_5
	.word 0x0100001c	@ FUNC_MUX_CTRL_6
	.word 0x0004800b	@ FUNC_MUX_CTRL_7
	.word 0x10001200	@ FUNC_MUX_CTRL_8
	.word 0x01201012	@ FUNC_MUX_CTRL_9
	.word 0x02082248	@ FUNC_MUX_CTRL_A
	.word 0x00000248	@ FUNC_MUX_CTRL_B
	.word 0x12240000	@ FUNC_MUX_CTRL_C
	.word 0x00002000	@ FUNC_MUX_CTRL_D
	.word 0x00000000	@ PULL_DWN_CTRL_0
	.word 0x00000800	@ PULL_DWN_CTRL_1
	.word 0x01801000	@ PULL_DWN_CTRL_2
	.word 0x00000000	@ PULL_DWN_CTRL_3
	.word 0x00000000	@ GATE_INH_CTRL_0
	.word 0x00000000	@ VOLTAGE_CTRL_0
	.word 0x00000000	@ TEST_DBG_CTRL_0
	.word 0x00000006	@ MOD_CONF_CTRL_0
	.word 0x0000eaef	@ COMP_MODE_CTRL_0

MUX_CONFIG_OFFSETS:
	.align 1
	.byte 0x00		@ FUNC_MUX_CTRL_0
	.byte 0x04		@ FUNC_MUX_CTRL_1
	.byte 0x08		@ FUNC_MUX_CTRL_2
	.byte 0x10		@ FUNC_MUX_CTRL_3
	.byte 0x14		@ FUNC_MUX_CTRL_4
	.byte 0x18		@ FUNC_MUX_CTRL_5
	.byte 0x1c		@ FUNC_MUX_CTRL_6
	.byte 0x20		@ FUNC_MUX_CTRL_7
	.byte 0x24		@ FUNC_MUX_CTRL_8
	.byte 0x28		@ FUNC_MUX_CTRL_9
	.byte 0x2c		@ FUNC_MUX_CTRL_A
	.byte 0x30		@ FUNC_MUX_CTRL_B
	.byte 0x34		@ FUNC_MUX_CTRL_C
	.byte 0x38		@ FUNC_MUX_CTRL_D
	.byte 0x40		@ PULL_DWN_CTRL_0
	.byte 0x44		@ PULL_DWN_CTRL_1
	.byte 0x48		@ PULL_DWN_CTRL_2
	.byte 0x4c		@ PULL_DWN_CTRL_3
	.byte 0x50		@ GATE_INH_CTRL_0
	.byte 0x60		@ VOLTAGE_CTRL_0
	.byte 0x70		@ TEST_DBG_CTRL_0
	.byte 0x80		@ MOD_CONF_CTRL_0
	.byte 0x0c		@ COMP_MODE_CTRL_0
	.byte 0xff

.globl lowlevel_init
lowlevel_init:
	/* Improve performance a bit... */
	mrc	p15, 0, r1, c0, c0, 0		@ read C15 ID register
	mrc	p15, 0, r1, c0, c0, 1		@ read C15 Cache information register
	mrc	p15, 0, r1, c1, c0, 0		@ read C15 Control register
	orr	r1, r1, #0x1000			@ enable I-cache, map interrupt vector 0xffff0000
	mcr	p15, 0, r1, c1, c0, 0		@ write C15 Control register
	mov	r1, #0x00
	mcr	p15, 0, r1, c7, c5, 0		@ Flush I-cache
	nop
	nop
	nop
	nop

	/* Setup clocking mode */
	ldr	r0, OMAP5910_MPU_CLKM_BASE	@ prepare base of CLOCK unit
	ldrh	r1, [r0, #0x18]			@ get reset status
	bic	r1, r1, #(7 << 11)		@ clear clock select
	orr	r1, r1, #(2 << 11)		@ set synchronous scalable
	mov	r2, #0				@ set wait counter to 100 clock cycles

icache_loop:
	cmp	r2, #0x01
	streqh	r1, [r0, #0x18]
	add	r2, r2, #0x01
	cmp	r2, #0x10
	bne	icache_loop
	nop

	/* Setup clock divisors */
	ldr	r0, OMAP5910_MPU_CLKM_BASE	@ base of CLOCK unit
	ldr	r1, _OMAP5910_ARM_CKCTL
	orr	r1, r1, #0x2000			@ enable DSP clock
	strh	r1, [r0, #0x00]			@ setup clock divisors

	/* Setup DPLL to generate requested freq */
	ldr	r0, OMAP5910_DPLL1_BASE		@ base of DPLL1 register
	mov	r1, #0x0010			@ set PLL_ENABLE
	orr	r1, r1, #0x2000			@ set IOB to new locking
	orr	r1, r1, #(OMAP5910_DPLL_MUL << 7) @ setup multiplier CLKREF
	orr	r1, r1, #(OMAP5910_DPLL_DIV << 5) @ setup divider CLKREF
	strh	r1, [r0]			@ write

locking:
	ldrh	r1, [r0]			@ get DPLL value
	tst	r1, #0x01
	beq	locking				@ while LOCK not set

	/* Enable clock */
	ldr	r0, OMAP5910_MPU_CLKM_BASE	@ base of CLOCK unit
	mov	r1, #(1 << 10)			@ disable idle mode do not check
						@ nWAKEUP pin, other remain active
	strh	r1, [r0, #0x04]
	ldr	r1, _OMAP5910_ARM_EN_CLK
	strh	r1, [r0, #0x08]
	mov	r1, #0x003f			@ FLASH.RP not enabled in idle and
						@ max delayed ( 32 x CLKIN )
	strh	r1, [r0, #0x0c]

	/* Configure 5910 pins functions to match our board. */
	ldr     r0, MUX_CONFIG_BASE
	adr	r1, MUX_CONFIG_VALUES
	adr	r2, MUX_CONFIG_OFFSETS
next_mux_cfg:
	ldrb	r3, [r2], #1
	ldr	r4, [r1], #4
	cmp	r3, #0xff
	strne	r4, [r0, r3]
	bne	next_mux_cfg

	/* Configure GPIO pins (also disables Green LED) */
	ldr	r0, OMAP5910_GPIO_BASE
	ldr	r1, GPIO_OUTPUT
	strh    r1, [r0, #0x04]
	ldr	r1, GPIO_DIRECTION
	strh	r1, [r0, #0x08]

	/* EnablePeripherals */
	ldr	r0, OMAP5910_MPU_CLKM_BASE	@ CLOCK unit
	mov	r1, #0x0001			@ Peripheral enable
	strh	r1, [r0, #0x14]

	/* Program LED Pulse Generator */
	ldr	r0, OMAP5910_LPG1_BASE		@ 1st LED Pulse Generator
	mov	r1, #0x7F			@ Set obscure frequency in
	strb	r1, [r0, #0x00]			@ LCR
	mov	r1, #0x01			@ Enable clock (CLK_EN) in
	strb    r1, [r0, #0x04]			@ PMR

	/* TIPB Lock UART1 */
	ldr	r0, OMAP5910_TIPB_SWITCHES_BASE	@ prepare base of TIPB switches
	mov	r1, #1				@ ARM allocated
	strh	r1, [r0,#0x04]			@ clear IRQ line and status bits
	strh	r1, [r0,#0x00]
	ldrh	r1, [r0,#0x04]

	/* Disable watchdog */
	ldr	r0, OMAP5910_MPU_WD_TIMER_BASE
	mov	r1, #0xf5
	strh	r1, [r0, #0x8]
	mov	r1, #0xa0
	strh	r1, [r0, #0x8]

	/* Enable MCLK */
	ldr	r0, OMAP5910_ULPD_PWR_MNG_BASE
	mov	r1, #0x6
	strh	r1, [r0, #0x34]
	strh	r1, [r0, #0x34]

	/* Setup clock divisors */
	ldr	r0, OMAP5910_ULPD_PWR_MNG_BASE	@ base of ULDPL DPLL1 register

	mov	r1, #0x0010			@ set PLL_ENABLE
	orr	r1, r1, #0x2000			@ set IOB to new locking
	strh	r1, [r0]			@ write

ulocking:
	ldrh	r1, [r0]			@ get DPLL value
	tst	r1, #1
	beq	ulocking			@ while LOCK not set

	/* EMIF init */
	ldr	r0, OMAP5910_MPU_TC_BASE
	ldrh	r1, [r0, #0x0c]			@ EMIFS_CONFIG_REG
	bic	r1, r1, #0x0c			@ pwr down disabled, flash WP
	orr	r1, r1, #0x01
	str	r1, [r0, #0x0c]

	ldr	r1, VAL_EMIFS_CS0_CONFIG
	str	r1, [r0, #0x10]			@ EMIFS_CS0_CONFIG
	ldr	r1, VAL_EMIFS_CS1_CONFIG
	str	r1, [r0, #0x14]			@ EMIFS_CS1_CONFIG
	ldr	r1, VAL_EMIFS_CS2_CONFIG
	str	r1, [r0, #0x18]			@ EMIFS_CS2_CONFIG
	ldr	r1, VAL_EMIFS_CS3_CONFIG
	str	r1, [r0, #0x1c]			@ EMIFS_CS3_CONFIG
	ldr	r1, VAL_EMIFS_DYN_WAIT
	str	r1, [r0, #0x40]			@ EMIFS_CFG_DYN_WAIT

	/* Setup SDRAM */
	ldr	r1, VAL_EMIFF_SDRAM_CONFIG
	str	r1, [r0, #0x20]			@ EMIFF_SDRAM_CONFIG
	ldr	r1, VAL_EMIFF_SDRAM_CONFIG2
	str	r1, [r0, #0x3c]			@ EMIFF_SDRAM_CONFIG2
	ldr	r1, VAL_EMIFF_MRS
	str	r1, [r0, #0x24]			@ EMIFF_MRS
	/* SDRAM needs 100us to stabilize */
	mov	r0, #0x4000
sdelay:
	subs	r0, r0, #0x1
	bne 	sdelay

	/* back to arch calling code */
	mov	pc, lr
.end
