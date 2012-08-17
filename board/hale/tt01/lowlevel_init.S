/*
 * (C) Copyright 2009 Magnus Lilja <lilja.magnus@gmail.com>
 * (C) Copyright 2011 Helmut Raiger <helmut.raiger@hale.at>
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
#include <asm/arch/imx-regs.h>
#include <asm/macro.h>

.globl lowlevel_init
lowlevel_init:
	/* Also setup the Peripheral Port Remap register inside the core */
	ldr	r0, =ARM_PPMRR      /* start from AIPS 2GB region */
	mcr	p15, 0, r0, c15, c2, 4
	mov	pc, lr
