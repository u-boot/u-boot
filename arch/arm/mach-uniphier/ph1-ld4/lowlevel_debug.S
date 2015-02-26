/*
 * On-chip UART initializaion for low-level debugging
 *
 * Copyright (C) 2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/linkage.h>
#include <asm/arch/sg-regs.h>

#define UART_CLK		36864000
#include <asm/arch/debug-uart.S>

ENTRY(setup_lowlevel_debug)
		init_debug_uart	r0, r1, r2

		/* UART Port 0 */
		set_pinsel	85, 1, r0, r1
		set_pinsel	88, 1, r0, r1

		ldr		r0, =SG_IECTRL
		ldr		r1, [r0]
		orr		r1, r1, #1
		str		r1, [r0]

		mov		pc, lr
ENDPROC(setup_lowlevel_debug)
