/*
 * On-chip UART initializaion for low-level debugging
 *
 * Copyright (C) 2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/linkage.h>
#include <mach/sc-regs.h>
#include <mach/sg-regs.h>

#define UART_CLK		73728000
#include <mach/debug-uart.S>

ENTRY(setup_lowlevel_debug)
		/* UART Port 0 */
		sg_set_pinsel	127, 0, 4, 8, r0, r1
		sg_set_pinsel	128, 0, 4, 8, r0, r1

		ldr		r0, =SG_LOADPINCTRL
		mov		r1, #1
		str		r1, [r0]

		ldr		r0, =SC_CLKCTRL
		ldr		r1, [r0]
		orr		r1, r1, #SC_CLKCTRL_CEN_PERI
		str		r1, [r0]

		init_debug_uart	r0, r1, r2

		mov		pc, lr
ENDPROC(setup_lowlevel_debug)
