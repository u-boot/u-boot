/*
 * On-chip UART initializaion for low-level debugging
 *
 * Copyright (C) 2014-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/linkage.h>
#include <mach/bcu-regs.h>
#include <mach/sc-regs.h>
#include <mach/sg-regs.h>

#define UART_CLK		36864000
#include <mach/debug-uart.S>

ENTRY(setup_lowlevel_debug)
		ldr		r0, =BCSCR5
		ldr		r1, =0x24440000
		str		r1, [r0]

		ldr		r0, =SC_CLKCTRL
		ldr		r1, [r0]
		orr		r1, r1, #SC_CLKCTRL_CEN_PERI
		str		r1, [r0]

		init_debug_uart	r0, r1, r2

		set_pinsel	63, 0, r0, r1
		set_pinsel	64, 1, r0, r1

		mov		pc, lr
ENDPROC(setup_lowlevel_debug)
