/*
 * On-chip UART initializaion for low-level debugging
 *
 * Copyright (C) 2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/linkage.h>
#include <mach/sg-regs.h>

#define UART_CLK		80000000
#include <mach/debug-uart.S>

ENTRY(setup_lowlevel_debug)
		init_debug_uart	r0, r1, r2

		/* UART Port 0 */
		set_pinsel	70, 3, r0, r1
		set_pinsel	71, 3, r0, r1

		ldr		r0, =SG_IECTRL
		ldr		r1, [r0]
		orr		r1, r1, #1
		str		r1, [r0]

		mov		pc, lr
ENDPROC(setup_lowlevel_debug)
