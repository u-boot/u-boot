/*
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
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
.section ".text.init", "x"

#include <linux/linkage.h>

.macro init_arm_errata
	/* ARM erratum ID #743622 */
	mrc	p15, 0, r10, c15, c0, 1		/* read diagnostic register */
	orr	r10, r10, #1 << 6		/* set bit #6 */
	/* ARM erratum ID #751472 */
	orr	r10, r10, #1 << 11		/* set bit #11 */
	mcr	p15, 0, r10, c15, c0, 1		/* write diagnostic register */
.endm

ENTRY(lowlevel_init)
	init_arm_errata
	mov pc, lr
ENDPROC(lowlevel_init)
