/*
 * Low-level initialization for EP93xx
 *
 * Copyright (C) 2009 Matthias Kaehlcke <matthias@kaehlcke.net>
 *
 * Copyright (C) 2006 Dominic Rath <Dominic.Rath@gmx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <version.h>
#include <asm/arch/ep93xx.h>

.globl lowlevel_init
lowlevel_init:
	/* backup return address */
	ldr r1, =SYSCON_SCRATCH0
	str lr, [r1]

	/* Turn on both LEDs */
	bl red_led_on
	bl green_led_on

	/* Configure flash wait states before we switch to the PLL */
	bl flash_cfg

	/* Set up PLL */
	bl pll_cfg

	/* Turn off the Green LED and leave the Red LED on */
	bl green_led_off

	/* Setup SDRAM */
	bl sdram_cfg

	/* Turn on Green LED, Turn off the Red LED */
	bl green_led_on
	bl red_led_off

	/* FIXME: we use async mode for now */
	mrc p15, 0, r0, c1, c0, 0
	orr r0, r0, #0xc0000000
	mcr p15, 0, r0, c1, c0, 0

	/* restore return address */
	ldr r1, =SYSCON_SCRATCH0
	ldr lr, [r1]

	mov pc, lr
