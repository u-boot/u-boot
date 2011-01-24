/*
 * Lowlevel setup for universal board based on S5PC210
 *
 * Copyright (C) 2010 Samsung Electronics
 * Kyungmin Park <kyungmin.park@samsung.com>
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
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>

/*
 * Register usages:
 *
 * r5 has zero always
 * r7 has GPIO part1 base 0x11400000
 * r6 has GPIO part2 base 0x11000000
 */

	.globl lowlevel_init
lowlevel_init:
	mov	r11, lr

	/* r5 has always zero */
	mov	r5, #0

	ldr	r7, =S5PC210_GPIO_PART1_BASE
	ldr	r6, =S5PC210_GPIO_PART2_BASE

	/* System Timer */
	ldr	r0, =S5PC210_SYSTIMER_BASE
	ldr	r1, =0x5000
	str	r1, [r0, #0x0]
	ldr	r1, =0xffffffff
	str	r1, [r0, #0x8]
	ldr	r1, =0x49
	str	r1, [r0, #0x4]

	/* PMIC manual reset */
	/* nPOWER: XEINT_23: GPX2[7] */
	add	r0, r6, #0xC40			@ S5PC210_GPIO_X2_OFFSET
	ldr	r1, [r0, #0x0]
	bic	r1, r1, #(0xf << 28)		@ 28 = 7 * 4-bit
	orr	r1, r1, #(0x1 << 28)		@ Output
	str	r1, [r0, #0x0]

	ldr	r1, [r0, #0x4]
	orr	r1, r1, #(1 << 7)		@ 7 = 7 * 1-bit
	str	r1, [r0, #0x4]

	/* init system clock */
	bl	system_clock_init

	/* Disable Watchdog */
	ldr	r0, =S5PC210_WATCHDOG_BASE		@0x10060000
	str	r5, [r0]

	/* UART */
	bl	uart_asm_init

	/* PMU init */
	bl	system_power_init

	bl	tzpc_init

	mov	lr, r11
	mov	pc, lr
	nop
	nop
	nop

/*
 * uart_asm_init: Initialize UART's pins
 */
uart_asm_init:
	/*
	 * setup UART0-UART4 GPIOs (part1)
	 * GPA1CON[3] = I2C_3_SCL (3)
	 * GPA1CON[2] = I2C_3_SDA (3)
	 */
	mov	r0, r7
	ldr	r1, =0x22222222
	str	r1, [r0, #0x00]			@ S5PC210_GPIO_A0_OFFSET
	ldr	r1, =0x00223322
	str	r1, [r0, #0x20]			@ S5PC210_GPIO_A1_OFFSET

	/* UART_SEL GPY4[7] (part2) at S5PC210 */
	add	r0, r6, #0x1A0			@ S5PC210_GPIO_Y4_OFFSET
	ldr	r1, [r0, #0x0]
	bic	r1, r1, #(0xf << 28)		@ 28 = 7 * 4-bit
	orr	r1, r1, #(0x1 << 28)
	str	r1, [r0, #0x0]

	ldr	r1, [r0, #0x8]
	bic	r1, r1, #(0x3 << 14)		@ 14 = 7 * 2-bit
	orr	r1, r1, #(0x3 << 14)		@ Pull-up enabled
	str	r1, [r0, #0x8]

	ldr	r1, [r0, #0x4]
	orr	r1, r1, #(1 << 7)		@ 7 = 7 * 1-bit
	str	r1, [r0, #0x4]

	mov	pc, lr
	nop
	nop
	nop

system_clock_init:
	ldr	r0, =S5PC210_CLOCK_BASE

	/* APLL(1), MPLL(1), CORE(0), HPM(0) */
	ldr	r1, =0x0101
	ldr	r2, =0x14200			@ CLK_SRC_CPU
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b

	/*
	 * CLK_SRC_TOP0
	 * MUX_ONENAND_SEL[28]	0: DOUT133, 1: DOUT166
	 * MUX_VPLL_SEL[8]	0: FINPLL,  1: FOUTVPLL
	 * MUX_EPLL_SEL[4]	0: FINPLL,  1: FOUTEPLL
	 */
	ldr	r1, =0x10000110
	ldr	r2, =0x0C210			@ CLK_SRC_TOP
	str	r1, [r0, r2]

	/* SATA: SCLKMPLL(0), MMC[0:4]: SCLKMPLL(6) */
	ldr	r1, =0x0066666
	ldr	r2, =0x0C240			@ CLK_SRC_FSYS
	str	r1, [r0, r2]
	/* UART[0:5], PWM: SCLKMPLL(6) */
	ldr	r1, =0x6666666
	ldr	r2, =0x0C250			@ CLK_SRC_PERIL0_OFFSET
	str	r1, [r0, r2]

	/* CPU0: CORE, COREM0, COREM1, PERI, ATB, PCLK_DBG, APLL */
	ldr	r1, =0x0133730
	ldr	r2, =0x14500			@ CLK_DIV_CPU0
	str	r1, [r0, r2]
	/* CPU1: COPY, HPM */
	ldr	r1, =0x03
	ldr	r2, =0x14504			@ CLK_DIV_CPU1
	str	r1, [r0, r2]
	/* DMC0: ACP, ACP_PCLK, DPHY, DMC, DMCD, DMCP, COPY2 CORE_TIMER */
	ldr	r1, =0x13111113
	ldr	r2, =0x10500			@ CLK_DIV_DMC0
	str	r1, [r0, r2]
	/* DMC1: PWI, DVSEM, DPM */
	ldr	r1, =0x01010100
	ldr	r2, =0x10504			@ CLK_DIV_DMC1
	str	r1, [r0, r2]
	/* LEFTBUS: GDL, GPL */
	ldr	r1, =0x13
	ldr	r2, =0x04500			@ CLK_DIV_LEFTBUS
	str	r1, [r0, r2]
	/* RIGHHTBUS: GDR, GPR */
	ldr	r1, =0x13
	ldr	r2, =0x08500			@ CLK_DIV_RIGHTBUS
	str	r1, [r0, r2]
	/*
	 * CLK_DIV_TOP
	 * ONENAND_RATIOD[18:16]: 0 SCLK_ONENAND = MOUTONENAND / (n + 1)
	 * ACLK_200, ACLK_100, ACLK_160, ACLK_133,
	 */
	ldr	r1, =0x00005473
	ldr	r2, =0x0C510			@ CLK_DIV_TOP
	str	r1, [r0, r2]
	/* MMC[0:1] */
	ldr	r1, =0x000f000f			/* 800(MPLL) / (15 + 1) */
	ldr	r2, =0x0C544			@ CLK_DIV_FSYS1
	str	r1, [r0, r2]
	/* MMC[2:3] */
	ldr	r1, =0x000f000f			/* 800(MPLL) / (15 + 1) */
	ldr	r2, =0x0C548			@ CLK_DIV_FSYS2
	str	r1, [r0, r2]
	/* MMC4 */
	ldr	r1, =0x000f			/* 800(MPLL) / (15 + 1) */
	ldr	r2, =0x0C54C			@ CLK_DIV_FSYS3
	str	r1, [r0, r2]
	/* UART[0:5] */
	ldr	r1, =0x774777
	ldr	r2, =0x0C550			@ CLK_DIV_PERIL0
	str	r1, [r0, r2]
	/* SLIMBUS: ???, PWM */
	ldr	r1, =0x8
	ldr	r2, =0x0C55C			@ CLK_DIV_PERIL3
	str	r1, [r0, r2]

	/* PLL Setting */
	ldr	r1, =0x1C20
	ldr	r2, =0x14000			@ APLL_LOCK
	str	r1, [r0, r2]
	ldr	r2, =0x14008			@ MPLL_LOCK
	str	r1, [r0, r2]
	ldr	r2, =0x0C010			@ EPLL_LOCK
	str	r1, [r0, r2]
	ldr	r2, =0x0C020			@ VPLL_LOCK
	str	r1, [r0, r2]

	/* APLL */
	ldr	r1, =0x8000001c
	ldr	r2, =0x14104			@ APLL_CON1
	str	r1, [r0, r2]
	ldr	r1, =0x80c80601			@ 800MHz
	ldr	r2, =0x14100			@ APLL_CON0
	str	r1, [r0, r2]
	/* MPLL */
	ldr	r1, =0x8000001C
	ldr	r2, =0x1410C			@ MPLL_CON1
	str	r1, [r0, r2]
	ldr	r1, =0x80c80601			@ 800MHz
	ldr	r2, =0x14108			@ MPLL_CON0
	str	r1, [r0, r2]
	/* EPLL */
	ldr	r1, =0x0
	ldr	r2, =0x0C114			@ EPLL_CON1
	str	r1, [r0, r2]
	ldr	r1, =0x80300302			@ 96MHz
	ldr	r2, =0x0C110			@ EPLL_CON0
	str	r1, [r0, r2]
	/* VPLL */
	ldr	r1, =0x11000400
	ldr	r2, =0x0C124			@ VPLL_CON1
	str	r1, [r0, r2]
	ldr	r1, =0x80350302			@ 108MHz
	ldr	r2, =0x0C120			@ VPLL_CON0
	str	r1, [r0, r2]

	/*
	 * SMMUJPEG[11], JPEG[6], CSIS1[5]		: 0111 1001
	 * Turn off all
	 */
	ldr	r1, =0xFFF80000
	ldr	r2, =0x0C920			@ CLK_GATE_IP_CAM
	str	r1, [r0, r2]

	/* Turn off all */
	ldr	r1, =0xFFFFFFC0
	ldr	r2, =0x0C924			@ CLK_GATE_IP_VP
	str	r1, [r0, r2]

	/* Turn off all */
	ldr	r1, =0xFFFFFFE0
	ldr	r2, =0x0C928			@ CLK_GATE_IP_MFC
	str	r1, [r0, r2]

	/* Turn off all */
	ldr	r1, =0xFFFFFFFC
	ldr	r2, =0x0C92C			@ CLK_GATE_IP_G3D
	str	r1, [r0, r2]

	/* Turn off all */
	ldr	r1, =0xFFFFFC00
	ldr	r2, =0x0C930			@ CLK_GATE_IP_IMAGE
	str	r1, [r0, r2]

	/* DSIM0[3], MDNIE0[2], MIE0[1]			: 0001 */
	ldr	r1, =0xFFFFFFF1
	ldr	r2, =0x0C934			@ CLK_GATE_IP_LCD0
	str	r1, [r0, r2]

	/* Turn off all */
	ldr	r1, =0xFFFFFFC0
	ldr	r2, =0x0C938			@ CLK_GATE_IP_LCD1
	str	r1, [r0, r2]

	/*
	 * SMMUPCIE[18], NFCON[16]			: 1111 1010
	 * PCIE[14],  SATA[10], SDMMC43[9:8]		: 1011 1000
	 * SDMMC1[6], TSI[4], SATAPHY[3], PCIEPHY[2]	: 1010 0011
	 */
	ldr	r1, =0xFFFAB8A3
	ldr	r2, =0x0C940			@ CLK_GATE_IP_FSYS
	str	r1, [r0, r2]

	/* Turn off all */
	ldr	r1, =0xFFFFFFFC
	ldr	r2, =0x0C94C			@ CLK_GATE_IP_GPS
	str	r1, [r0, r2]

	/*
	 * AC97[27], SPDIF[26], SLIMBUS[25]		: 1111 0001
	 * I2C2[8]					: 1111 1110
	 */
	ldr	r1, =0xF1FFFEFF
	ldr	r2, =0x0C950			@ CLK_GATE_IP_PERIL
	str	r1, [r0, r2]

	/*
	 * KEYIF[16]					: 1111 1110
	 */
	ldr	r1, =0xFFFEFFFF
	ldr	r2, =0x0C960			@ CLK_GATE_IP_PERIR
	str	r1, [r0, r2]

	/* LCD1[5], G3D[3], MFC[2], TV[1]		: 1101 0001 */
	ldr	r1, =0xFFFFFFD1
	ldr	r2, =0x0C970			@ CLK_GATE_BLOCK
	str	r1, [r0, r2]
	mov	pc, lr
	nop
	nop
	nop

system_power_init:
	ldr	r0, =S5PC210_POWER_BASE		@ 0x10020000

	ldr	r2, =0x330C			@ PS_HOLD_CONTROL
	ldr	r1, [r0, r2]
	orr	r1, r1, #(0x3 << 8)		@ Data High, Output En
	str	r1, [r0, r2]

	/* Power Down */
	add	r2, r0, #0x3000
	str	r5, [r2, #0xC20]		@ TV_CONFIGURATION
	str	r5, [r2, #0xC40]		@ MFC_CONFIGURATION
	str	r5, [r2, #0xC60]		@ G3D_CONFIGURATION
	str	r5, [r2, #0xCA0]		@ LCD1_CONFIGURATION
	str	r5, [r2, #0xCE0]		@ GPS_CONFIGURATION

	mov	pc, lr
	nop
	nop
	nop

tzpc_init:
	ldr	r0, =0x10110000
	mov	r1, #0x0
	str	r1, [r0]
	mov	r1, #0xff
	str	r1, [r0, #0x0804]
	str	r1, [r0, #0x0810]
	str	r1, [r0, #0x081C]
	str	r1, [r0, #0x0828]

	ldr	r0, =0x10120000
	mov	r1, #0x0
	str	r1, [r0]
	mov	r1, #0xff
	str	r1, [r0, #0x0804]
	str	r1, [r0, #0x0810]
	str	r1, [r0, #0x081C]
	str	r1, [r0, #0x0828]

	ldr	r0, =0x10130000
	mov	r1, #0x0
	str	r1, [r0]
	mov	r1, #0xff
	str	r1, [r0, #0x0804]
	str	r1, [r0, #0x0810]
	str	r1, [r0, #0x081C]
	str	r1, [r0, #0x0828]

	ldr	r0, =0x10140000
	mov	r1, #0x0
	str	r1, [r0]
	mov	r1, #0xff
	str	r1, [r0, #0x0804]
	str	r1, [r0, #0x0810]
	str	r1, [r0, #0x081C]
	str	r1, [r0, #0x0828]

	ldr	r0, =0x10150000
	mov	r1, #0x0
	str	r1, [r0]
	mov	r1, #0xff
	str	r1, [r0, #0x0804]
	str	r1, [r0, #0x0810]
	str	r1, [r0, #0x081C]
	str	r1, [r0, #0x0828]

	mov	pc, lr
