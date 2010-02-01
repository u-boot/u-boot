/*
 * Low-level initialization for EP93xx
 *
 * Copyright (C) 2009 Matthias Kaehlcke <matthias@kaehlcke.net>
 *
 * Copyright (C) 2006 Dominic Rath <Dominic.Rath@gmx.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <version.h>
#include <asm/arch/ep93xx.h>

.globl lowlevel_init
lowlevel_init:
	/* backup return address */
	ldr r1, =SYSCON_SCRATCH0
	str lr, [r1]

	/* Turn on both LEDs */
	bl red_LED_on
	bl green_LED_on

	/* Configure flash wait states before we switch to the PLL */
	bl flash_cfg

	/* Set up PLL */
	bl pll_cfg

	/* Turn off the Green LED and leave the Red LED on */
	bl green_LED_off

	/* Setup SDRAM */
	bl sdram_cfg

	/* Turn on Green LED, Turn off the Red LED */
	bl green_LED_on
	bl red_LED_off

	/* FIXME: we use async mode for now */
	mrc p15, 0, r0, c1, c0, 0
	orr r0, r0, #0xc0000000
	mcr p15, 0, r0, c1, c0, 0

	/* restore return address */
	ldr r1, =SYSCON_SCRATCH0
	ldr lr, [r1]

	mov pc, lr
