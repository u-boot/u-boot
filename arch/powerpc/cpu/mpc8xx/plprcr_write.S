/*
 * (C) Copyright 2004
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <mpc8xx.h>
#include <ppc_asm.tmpl>
#include <asm/cache.h>

#define CACHE_CMD_ENABLE	0x02000000
#define CACHE_CMD_DISABLE	0x04000000
#define CACHE_CMD_LOAD_LOCK	0x06000000
#define CACHE_CMD_UNLOCK_LINE	0x08000000
#define CACHE_CMD_UNLOCK_ALL	0x0A000000
#define CACHE_CMD_INVALIDATE	0x0C000000
#define SPEED_PLPRCR_WAIT_5CYC	150
#define _CACHE_ALIGN_SIZE	16


	.text
	.align 2
	.globl plprcr_write_866

/*
 * void plprcr_write_866 (long plprcr)
 * Write PLPRCR, including workaround for device errata SIU4 and SIU9.
 */

plprcr_write_866:
	mfspr	r10, LR		/* save the Link Register value */

	/* turn instruction cache on (no MMU required for instructions)
	 */
	lis	r4, CACHE_CMD_ENABLE@h
	ori	r4, r4, CACHE_CMD_ENABLE@l
	mtspr	IC_CST, r4
	isync

	/* clear IC_CST error bits
	 */
	mfspr	r4, IC_CST

	bl	plprcr_here

plprcr_here:
	mflr	r5

	/* calculate relocation offset
	 */
	lis	r4, plprcr_here@h
	ori	r4, r4, plprcr_here@l
	sub	r5, r5, r4

	/* calculate first address of this function
	 */
	lis	r6, plprcr_write_866@h
	ori	r6, r6, plprcr_write_866@l
	add	r6, r6, r5

	/* calculate end address of this function
	 */
	lis	r7, plprcr_end@h
	ori	r7, r7, plprcr_end@l
	add	r7, r7, r5

	/* load and lock code addresses
	 */
	mr	r5, r6

plprcr_loop:
	mtspr	IC_ADR, r5
	addi	r5, r5, _CACHE_ALIGN_SIZE	/* increment by one line */

	lis	r4, CACHE_CMD_LOAD_LOCK@h
	ori	r4, r4, CACHE_CMD_LOAD_LOCK@l
	mtspr	IC_CST, r4
	isync

	cmpw	r5, r7
	blt	plprcr_loop

	/* IC_CST error bits not evaluated
	 */

	/* switch PLPRCR
	 */
	mfspr	r4, IMMR		/* read IMMR */
	rlwinm	r4, r4, 0, 0, 15	/* only high 16 bits count */

	/* write sequence according to MPC866 Errata
	 */
	stw	r3, PLPRCR(r4)
	isync

	lis	r3, SPEED_PLPRCR_WAIT_5CYC@h
	ori	r3, r3, SPEED_PLPRCR_WAIT_5CYC@l

plprcr_wait:
	cmpwi	r3, 0
	beq	plprcr_wait_end
	nop
	subi	r3, r3, 1
	b	plprcr_wait

plprcr_wait_end:

	/* unlock instruction cache but leave it enabled
	 */
	lis	r4, CACHE_CMD_UNLOCK_ALL@h
	ori	r4, r4, CACHE_CMD_UNLOCK_ALL@l
	mtspr	IC_CST, r4
	isync

	mtspr	LR, r10		/* restore original Link Register value */
	blr

plprcr_end:
