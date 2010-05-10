/*
 *  Copyright (C) 2000	Murray Jensen <Murray.Jensen@cmst.csiro.au>
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
 * MA 02111-1307USA
 */

#include <config.h>
#include <command.h>
#include <74xx_7xx.h>
#include <version.h>

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>

#if defined(CONFIG_CMD_KGDB)

 /*
 * cache flushing routines for kgdb
 */

	.globl	kgdb_flush_cache_all
kgdb_flush_cache_all:
	lis	r3,0
	addis	r4,r0,0x0040
kgdb_flush_loop:
	lwz	r5,0(r3)
	addi	r3,r3,CONFIG_SYS_CACHELINE_SIZE
	cmp	0,0,r3,r4
	bne	kgdb_flush_loop
	SYNC
	mfspr	r3,1008
	ori	r3,r3,0x8800
	mtspr	1008,r3
	sync
	blr

	.globl	kgdb_flush_cache_range
kgdb_flush_cache_range:
	li	r5,CONFIG_SYS_CACHELINE_SIZE-1
	andc	r3,r3,r5
	subf	r4,r3,r4
	add	r4,r4,r5
	srwi.	r4,r4,CONFIG_SYS_CACHELINE_SHIFT
	beqlr
	mtctr	r4
	mr	r6,r3
1:	dcbst	0,r3
	addi	r3,r3,CONFIG_SYS_CACHELINE_SIZE
	bdnz	1b
	sync				/* wait for dcbst's to get to ram */
	mtctr	r4
2:	icbi	0,r6
	addi	r6,r6,CONFIG_SYS_CACHELINE_SIZE
	bdnz	2b
	SYNC
	blr

#endif
