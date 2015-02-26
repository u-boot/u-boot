/*
 * On-chip UART initializaion for low-level debugging
 *
 * Copyright (C) 2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/linkage.h>
#include <asm/arch/sc-regs.h>
#include <asm/arch/sg-regs.h>

#define UART_CLK		73728000
#include <asm/arch/debug-uart.S>

ENTRY(setup_lowlevel_debug)
		ldr		r0, =SC_CLKCTRL
		ldr		r1, [r0]
		orr		r1, r1, #SC_CLKCTRL_CLK_PERI
		str		r1, [r0]

		init_debug_uart	r0, r1, r2

		/* UART Port 0 */
		set_pinsel	127, 0, r0, r1
		set_pinsel	128, 0, r0, r1

		ldr		r0, =SG_LOADPINCTRL
		mov		r1, #1
		str		r1, [r0]

		ldr		r0, =SG_IECTRL
		ldr		r1, [r0]
		orr		r1, r1, #1
		str		r1, [r0]

		mov		pc, lr
ENDPROC(setup_lowlevel_debug)
