/*
 * On-chip UART initializaion for low-level debugging
 *
 * Copyright (C) 2014-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/serial_reg.h>
#include <linux/linkage.h>
#include <mach/bcu-regs.h>
#include <mach/sc-regs.h>
#include <mach/sg-regs.h>

#if !defined(CONFIG_DEBUG_SEMIHOSTING)
#include CONFIG_DEBUG_LL_INCLUDE
#endif

#define BAUDRATE		115200
#define DIV_ROUND(x, d)		(((x) + ((d) / 2)) / (d))

ENTRY(debug_ll_init)
	ldr		r0, =SG_REVISION
	ldr		r1, [r0]
	and		r1, r1, #SG_REVISION_TYPE_MASK
	mov		r1, r1, lsr #SG_REVISION_TYPE_SHIFT

#if defined(CONFIG_ARCH_UNIPHIER_PH1_SLD3)
#define PH1_SLD3_UART_CLK		36864000
	cmp		r1, #0x25
	bne		ph1_sld3_end

	sg_set_pinsel	64, 1, 4, 4, r0, r1	@ TXD0 -> TXD0

	ldr		r0, =BCSCR5
	ldr		r1, =0x24440000
	str		r1, [r0]

	ldr		r0, =SC_CLKCTRL
	ldr		r1, [r0]
	orr		r1, r1, #SC_CLKCTRL_CEN_PERI
	str		r1, [r0]

	ldr		r3, =DIV_ROUND(PH1_SLD3_UART_CLK, 16 * BAUDRATE)

	b		init_uart
ph1_sld3_end:
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_LD4)
#define PH1_LD4_UART_CLK		36864000
	cmp		r1, #0x26
	bne		ph1_ld4_end

	ldr		r0, =SG_IECTRL
	ldr		r1, [r0]
	orr		r1, r1, #1
	str		r1, [r0]

	sg_set_pinsel	88, 1, 8, 4, r0, r1	@ HSDOUT6 -> TXD0

	ldr		r3, =DIV_ROUND(PH1_LD4_UART_CLK, 16 * BAUDRATE)

	b		init_uart
ph1_ld4_end:
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_PRO4)
#define PH1_PRO4_UART_CLK		73728000
	cmp		r1, #0x28
	bne		ph1_pro4_end

	sg_set_pinsel	128, 0, 4, 8, r0, r1	@ TXD0 -> TXD0

	ldr		r0, =SG_LOADPINCTRL
	mov		r1, #1
	str		r1, [r0]

	ldr		r0, =SC_CLKCTRL
	ldr		r1, [r0]
	orr		r1, r1, #SC_CLKCTRL_CEN_PERI
	str		r1, [r0]

	ldr		r3, =DIV_ROUND(PH1_PRO4_UART_CLK, 16 * BAUDRATE)

	b		init_uart
ph1_pro4_end:
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_SLD8)
#define PH1_SLD8_UART_CLK		80000000
	cmp		r1, #0x29
	bne		ph1_sld8_end

	ldr		r0, =SG_IECTRL
	ldr		r1, [r0]
	orr		r1, r1, #1
	str		r1, [r0]

	sg_set_pinsel	70, 3, 8, 4, r0, r1	@ HSDOUT0 -> TXD0

	ldr		r3, =DIV_ROUND(PH1_SLD8_UART_CLK, 16 * BAUDRATE)

	b		init_uart
ph1_sld8_end:
#endif

init_uart:
	addruart	r0, r1, r2
	mov		r1, #UART_LCR_WLEN8 << 8
	str		r1, [r0, #0x10]
	str		r3, [r0, #0x24]

	mov		pc, lr
ENDPROC(debug_ll_init)
