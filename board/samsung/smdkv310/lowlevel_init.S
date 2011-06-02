/*
 * Lowlevel setup for SMDKV310 board based on S5PC210
 *
 * Copyright (C) 2011 Samsung Electronics
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

/*
 * Register usages:
 *
 * r5 has zero always
 * r7 has GPIO part1 base 0x11400000
 * r6 has GPIO part2 base 0x11000000
 */

#define MEM_DLLl_ON

_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE

	.globl lowlevel_init
lowlevel_init:
	push	{lr}

	/* r5 has always zero */
	mov	r5, #0
	ldr	r7, =S5PC210_GPIO_PART1_BASE
	ldr	r6, =S5PC210_GPIO_PART2_BASE

	/* check reset status  */
	ldr     r0, =(S5PC210_POWER_BASE + 0x81C)	@ INFORM7
	ldr     r1, [r0]

	/* AFTR wakeup reset */
	ldr	r2, =S5P_CHECK_DIDLE
	cmp	r1, r2
	beq	exit_wakeup

	/* Sleep wakeup reset */
	ldr	r2, =S5P_CHECK_SLEEP
	cmp	r1, r2
	beq	wakeup_reset

	/*
	 * If U-boot is already running in ram, no need to relocate U-Boot.
	 * Memory controller must be configured before relocating U-Boot
	 * in ram.
	 */
	ldr	r0, =0x00ffffff		/* r0 <- Mask Bits*/
	bic	r1, pc, r0		/* pc <- current addr of code */
					/* r1 <- unmasked bits of pc */

	ldr	r2, _TEXT_BASE		/* r2 <- original base addr in ram */
	bic	r2, r2, r0		/* r2 <- unmasked bits of r2*/
	cmp     r1, r2                  /* compare r1, r2 */
	beq     1f			/* r0 == r1 then skip sdram init */

	/* init system clock */
	bl system_clock_init

	/* Memory initialize */
	bl mem_ctrl_asm_init

1:
	/* for UART */
	bl uart_asm_init
	bl tzpc_init
	pop	{pc}

wakeup_reset:
	bl system_clock_init
	bl mem_ctrl_asm_init
	bl tzpc_init

exit_wakeup:
	/* Load return address and jump to kernel */
	ldr     r0, =(S5PC210_POWER_BASE + 0x800)	@ INFORM0

	/* r1 = physical address of s5pc210_cpu_resume function */
	ldr	r1, [r0]

	/* Jump to kernel*/
	mov	pc, r1
	nop
	nop

/*
 * system_clock_init: Initialize core clock and bus clock.
 * void system_clock_init(void)
 */
system_clock_init:
	push	{lr}
	ldr	r0, =S5PC210_CLOCK_BASE

	/* APLL(1), MPLL(1), CORE(0), HPM(0) */
	ldr	r1, =0x0101
	ldr	r2, =0x14200			@CLK_SRC_CPU
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x10000
2:	subs	r1, r1, #1
	bne	2b

	ldr	r1, =0x00
	ldr	r2, =0x0C210			@CLK_SRC_TOP0
	str	r1, [r0, r2]

	ldr	r1, =0x00
	ldr	r2, =0x0C214			@CLK_SRC_TOP1_OFFSET
	str	r1, [r0, r2]

	/* DMC */
	ldr	r1, =0x00
	ldr	r2, =0x10200			@CLK_SRC_DMC_OFFSET
	str	r1, [r0, r2]

	/*CLK_SRC_LEFTBUS */
	ldr	r1, =0x00
	ldr	r2, =0x04200			@CLK_SRC_LEFTBUS_OFFSET
	str	r1, [r0, r2]

	/*CLK_SRC_RIGHTBUS */
	ldr	r1, =0x00
	ldr	r2, =0x08200			@CLK_SRC_RIGHTBUS_OFFSET
	str	r1, [r0, r2]

	/* SATA: SCLKMPLL(0), MMC[0:4]: SCLKMPLL(6) */
	ldr	r1, =0x066666
	ldr	r2, =0x0C240			@ CLK_SRC_FSYS
	str	r1, [r0, r2]

	/* UART[0:4], PWM: SCLKMPLL(6) */
	ldr	r1, =0x06666666
	ldr	r2, =0x0C250			@CLK_SRC_PERIL0_OFFSET
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x10000
3:	subs	r1, r1, #1
	bne	3b

	/*
	 * CLK_DIV_CPU0:
	 *
	 * PCLK_DBG_RATIO[20]	0x1
	 * ATB_RATIO[16]	0x3
	 * PERIPH_RATIO[12]	0x3
	 * COREM1_RATIO[8]	0x7
	 * COREM0_RATIO[4]	0x3
	 */
	ldr	r1, =0x0133730
	ldr	r2, =0x14500			@CLK_DIV_CPU0_OFFSET
	str	r1, [r0, r2]

	/* CLK_DIV_CPU1: COPY_RATIO [0]	0x3 */
	ldr	r1, =0x03
	ldr	r2, =0x14504			@CLK_DIV_CPU1_OFFSET
	str	r1, [r0, r2]

	/*
	 * CLK_DIV_DMC0:
	 *
	 * CORE_TIMERS_RATIO[28]	0x1
	 * COPY2_RATIO[24]	0x3
	 * DMCP_RATIO[20]	0x1
	 * DMCD_RATIO[16]	0x1
	 * DMC_RATIO[12]	0x1
	 * DPHY_RATIO[8]	0x1
	 * ACP_PCLK_RATIO[4]	0x1
	 * ACP_RATIO[0]		0x3
	 */
	ldr	r1, =0x13111113
	ldr	r2, =0x010500			@CLK_DIV_DMC0_OFFSET
	str	r1, [r0, r2]

	/*
	 * CLK_DIV_DMC1:
	 *
	 * DPM_RATIO[24]	0x1
	 * DVSEM_RATIO[16]	0x1
	 * PWI_RATIO[8]		0x1
	 */
	ldr	r1, =0x01010100
	ldr	r2, =0x010504			@CLK_DIV_DMC1_OFFSET
	str	r1, [r0, r2]

	/*
	 * CLK_DIV_LEFRBUS:
	 *
	 * GPL_RATIO[4]		0x1
	 * GDL_RATIO[0]		0x3
	 */
	ldr	r1, =0x013
	ldr	r2, =0x04500			@CLK_DIV_LEFTBUS_OFFSET
	str	r1, [r0, r2]

	/*
	 * CLK_DIV_RIGHTBUS:
	 *
	 * GPR_RATIO[4]		0x1
	 * GDR_RATIO[0]		0x3
	 */
	ldr	r1, =0x013
	ldr	r2, =0x08500			@CLK_DIV_RIGHTBUS_OFFSET
	str	r1, [r0, r2]

	/*
	 * CLK_DIV_TOP:
	 *
	 * ONENAND_RATIO[16]	0x0
	 * ACLK_133_RATIO[12]	0x5
	 * ACLK_160_RATIO[8]	0x4
	 * ACLK_100_RATIO[4]	0x7
	 * ACLK_200_RATIO[0]	0x3
	 */
	ldr	r1, =0x05473
	ldr	r2, =0x0C510			@CLK_DIV_TOP_OFFSET
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

	/* wait ?us */
	mov	r1, #0x10000
4:	subs	r1, r1, #1
	bne	4b

	/*
	 * CLK_DIV_PERIL0:
	 *
	 * UART5_RATIO[20]	8
	 * UART4_RATIO[16]	8
	 * UART3_RATIO[12]	8
	 * UART2_RATIO[8]	8
	 * UART1_RATIO[4]	8
	 * UART0_RATIO[0]	8
	 */
	ldr	r1, =0x774777
	ldr	r2, =0x0C550			@CLK_DIV_PERIL0_OFFSET
	str	r1, [r0, r2]

	/* SLIMBUS: ???, PWM */
	ldr	r1, =0x8
	ldr	r2, =0x0C55C			@ CLK_DIV_PERIL3
	str	r1, [r0, r2]

	/* Set PLL locktime */
	ldr	r1, =0x01C20
	ldr	r2, =0x014000			@APLL_LOCK_OFFSET
	str	r1, [r0, r2]
	ldr	r1, =0x01C20
	ldr	r2, =0x014008			@MPLL_LOCK_OFFSET
	str	r1, [r0, r2]
	ldr	r1, =0x01C20
	ldr	r2, =0x0C010			@EPLL_LOCK_OFFSET
	str	r1, [r0, r2]
	ldr	r1, =0x01C20
	ldr	r2, =0x0C020			@VPLL_LOCK_OFFSET
	str	r1, [r0, r2]

	/*
	 * APLL_CON1:
	 *
	 * APLL_AFC_ENB[31]	0x1
	 * APLL_AFC[0]		0xC
	 */
	ldr	r1, =0x8000000C
	ldr	r2, =0x014104			@APLL_CON1_OFFSET
	str	r1, [r0, r2]

	/*
	 * APLL_CON0:
	 *
	 * APLL_MDIV[16]	0xFA
	 * APLL_PDIV[8]		0x6
	 * APLL_SDIV[0]		0x1
	 */
	ldr	r1, =0x80FA0601
	ldr	r2, =0x014100			@APLL_CON0_OFFSET
	str	r1, [r0, r2]

	/*
	 * MPLL_CON1:
	 *
	 * MPLL_AFC_ENB[31]	0x1
	 * MPLL_AFC[0]		0x1C
	 */
	ldr	r1, =0x0000001C
	ldr	r2, =0x01410C			@MPLL_CON1_OFFSET
	str	r1, [r0, r2]

	/*
	 * MPLL_CON0:
	 *
	 * MPLL_MDIV[16]	0xC8
	 * MPLL_PDIV[8]		0x6
	 * MPLL_SDIV[0]		0x1
	 */
	ldr	r1, =0x80C80601
	ldr	r2, =0x014108			@MPLL_CON0_OFFSET
	str	r1, [r0, r2]

	/* EPLL */
	ldr	r1, =0x0
	ldr	r2, =0x0C114			@EPLL_CON1_OFFSET
	str	r1, [r0, r2]

	/*
	 * EPLL_CON0:
	 *
	 * EPLL_MDIV[16]	0x30
	 * EPLL_PDIV[8]		0x3
	 * EPLL_SDIV[0]		0x2
	 */
	ldr	r1, =0x80300302
	ldr	r2, =0x0C110			@EPLL_CON0_OFFSET
	str	r1, [r0, r2]

	/*
	 * VPLL_CON1:
	 *
	 * VPLL_MRR[24]		0x11
	 * VPLL_MFR[16]		0x0
	 * VPLL_K[0]		0x400
	 */
	ldr	r1, =0x11000400
	ldr	r2, =0x0C124			@VPLL_CON1_OFFSET
	str	r1, [r0, r2]

	/*
	 * VPLL_CON0:
	 *
	 * VPLL_MDIV[16]	0x35
	 * VPLL_PDIV[8]		0x3
	 * VPLL_SDIV[0]		0x2
	 */
	ldr	r1, =0x80350302
	ldr	r2, =0x0C120			@VPLL_CON0_OFFSET
	str	r1, [r0, r2]

	/* wait ?us */
	mov	r1, #0x30000
3:	subs	r1, r1, #1
	bne	3b

	pop	{pc}
/*
 * uart_asm_init: Initialize UART in asm mode, 115200bps fixed.
 * void uart_asm_init(void)
 */
	.globl uart_asm_init
uart_asm_init:

	/* setup UART0-UART3 GPIOs (part1) */
	mov	r0, r7
	ldr	r1, =0x22222222
	str	r1, [r0, #0x00]			@ S5PC210_GPIO_A0_OFFSET
	ldr	r1, =0x00222222
	str	r1, [r0, #0x20]			@ S5PC210_GPIO_A1_OFFSET

	ldr r0, =S5PC210_UART_BASE
	add r0, r0, #S5PC210_DEFAULT_UART_OFFSET

	ldr r1, =0x3C5
	str	r1, [r0, #0x4]
	ldr r1, =0x111
	str	r1, [r0, #0x8]
	ldr r1, =0x3
	str	r1, [r0, #0x0]
	ldr r1, =0x35
	str	r1, [r0, #0x28]
	ldr r1, =0x4
	str	r1, [r0, #0x2c]

	mov	pc, lr
	nop
	nop
	nop

/* Setting TZPC[TrustZone Protection Controller] */
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

	ldr	r0, =0x10160000
	mov	r1, #0x0
	str	r1, [r0]
	mov	r1, #0xff
	str	r1, [r0, #0x0804]
	str	r1, [r0, #0x0810]
	str	r1, [r0, #0x081C]
	str	r1, [r0, #0x0828]

	mov	pc, lr
