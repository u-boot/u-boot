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

SYSCON1:	.long	0x80000100
SYSCON2:	.long	0x80001100
SYSCON3:	.long	0x80002200
MEMCFG1:	.long	0x80000180
MEMCFG2:	.long	0x800001C0
SDCONF:		.long	0x80002300
SDRFPR:		.long	0x80002340

syscon1_val:	.long	0x00040100
syscon2_val:	.long	0x00000102
syscon3_val:	.long	0x0000020E
memcfg1_val:	.long	0x1f101710
memcfg2_mask:	.long	0x0000ffff @ only set lower 16 bits
memcfg2_val:	.long	0x00001f13 @ upper 16 bits are reserved for CS7 + CS6
sdrfpr_val:	.long	0x00000240
sdconf_val:	.long	0x00000522
/* setting up the memory */

.globl lowlevel_init
lowlevel_init:
	/*
	 * SYSCON1-3
	 */
	ldr	r0, SYSCON1
	ldr	r1, syscon1_val
	str	r1, [r0]

	ldr	r0, SYSCON2
	ldr	r1, syscon2_val
	str	r1, [r0]

	ldr	r0, SYSCON3
	ldr	r1, syscon3_val
	str	r1, [r0]

	/*
	 * MEMCFG1
	 */
	ldr	r0, MEMCFG1
	ldr	r1, memcfg1_val
	str	r1, [r0]

	/*
	 * MEMCFG2
	 */
	ldr	r0, MEMCFG2
	ldr	r2, [r0]
	ldr	r1, memcfg2_mask
	bic	r2, r2, r1
	ldr	r1, memcfg2_val
	orr	r2, r2, r1
	str	r2, [r0]

	/*
	 * SDRFPR,SDCONF
	 */
	ldr	r0, SDCONF
	ldr	r1, sdconf_val
	str	r1, [r0]

	ldr	r0, SDRFPR
	ldr	r1, sdrfpr_val
	str	r1, [r0]

	/* everything is fine now */
	mov	pc, lr
