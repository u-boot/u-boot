/*
 *  armboot - Startup Code for OMP2420/ARM1136 CPU-core
 *
 *  Copyright (c) 2004	Texas Instruments <r-woodruff2@ti.com>
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <garyj@denx.de>
 *  Copyright (c) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (c) 2003	Kshitij <kshitij@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#include <asm/arch/omap2420.h>

.globl reset_cpu
reset_cpu:
	ldr	r1, rstctl	/* get addr for global reset reg */
	mov	r3, #0x2	/* full reset pll+mpu */
	str	r3, [r1]	/* force reset */
	mov	r0, r0
_loop_forever:
	b	_loop_forever
rstctl:
	.word	PM_RSTCTRL_WKUP
