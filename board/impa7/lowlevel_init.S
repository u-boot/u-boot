/*
 * Memory Setup stuff - taken from ???
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


/* some parameters for the board */

SYSCON2:	.long	0x80001100
MEMCFG1:	.long	0x80000180
MEMCFG2:	.long	0x800001C0
DRFPR:		.long	0x80000200

syscon2_mask:	.long	0x00000004
memcfg1_val:	.long	0x160c1414
memcfg2_mask:	.long	0x0000ffff @ only set lower 16 bits
memcfg2_val:	.long	0x00000000 @ upper 16 bits are reserved for CS7 + CS6
drfpr_val:	.long	0x00000081
/* setting up the memory */

.globl lowlevel_init
lowlevel_init:
	/*
	 * DRFPR
	 * 64kHz DRAM refresh
	 */
	ldr	r0, DRFPR
	ldr	r1, drfpr_val
	str	r1, [r0]

	/*
	 * SYSCON2: clear bit 2, DRAM is 32 bits wide
	 */
	ldr	r0, SYSCON2
	ldr	r2, [r0]
	ldr	r1, syscon2_mask
	bic	r2, r2, r1
	str	r2, [r0]

	/*
	 * MEMCFG1
	 * Setting up Keyboard at CS3, 8 Bit, 3 Waitstates
	 * Setting up CS8900 (Ethernet) at CS2, 32 Bit, 5 Waitstates
	 * Setting up flash at CS0 and CS1, 32 Bit, 3 Waitstates
	 */
	ldr	r0, MEMCFG1
	ldr	r1, memcfg1_val
	str	r1, [r0]

	/*
	 * MEMCFG2
	 * Setting up ? with 0
	 *
	 */
	ldr	r0, MEMCFG2
	ldr	r2, [r0]
	ldr	r1, memcfg2_mask
	bic	r2, r2, r1
	ldr	r1, memcfg2_val
	orr	r2, r2, r1
	str	r2, [r0]

	/* everything is fine now */
	mov	pc, lr
