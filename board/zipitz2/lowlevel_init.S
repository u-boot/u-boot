/*
 * Aeronix Zipit Z2 Lowlevel Hardware Initialization
 *
 * Copyright (C) 2010 Marek Vasut <marek.vasut@gmail.com>
 *
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
#include <asm/arch/pxa-regs.h>
#include <asm/arch/macro.h>

.globl lowlevel_init
lowlevel_init:
	pxa_gpio_setup
	pxa_wait_ticks	0x8000
	pxa_mem_setup
	pxa_wakeup
	pxa_intr_setup
	pxa_clock_setup

	mov	pc, lr
