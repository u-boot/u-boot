/*
 * (C) Copyright 2000, 2001
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com.
 *  base on code by
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <ppc_asm.tmpl>
#include <ppc_defs.h>
#include <config.h>
#include <watchdog.h>

/*
 * unsigned long long get_ticks(void);
 *
 * read timebase as "long long"
 */
	.globl	get_ticks
get_ticks:
1:	mftbu	r3
	mftb	r4
	mftbu	r5
	cmp	0,r3,r5
	bne	1b
	blr

/*
 * Delay for a number of ticks
 */
	.globl	wait_ticks
wait_ticks:
	stwu	r1, -16(r1)
	mflr	r0		/* save link register */
	stw	r0, 20(r1)	/* Use r0 or GDB will be unhappy */
	stw	r14, 12(r1)	/* save used registers */
	stw	r15, 8(r1)
	mr	r14, r3		/* save tick count */
	bl	get_ticks	/* Get start time */

	/* Calculate end time */
	addc    r14, r4, r14	/* Compute end time lower */
	addze	r15, r3		/*     and end time upper */

	WATCHDOG_RESET		/* Trigger watchdog, if needed */
1:	bl	get_ticks	/* Get current time */
	subfc	r4, r4, r14	/* Subtract current time from end time */
	subfe.	r3, r3, r15
	bge	1b		/* Loop until time expired */

	lwz     r15, 8(r1)	/* restore saved registers */
	lwz     r14, 12(r1)
	lwz     r0, 20(r1)
	addi	r1,r1,16
	mtlr	r0
	blr
