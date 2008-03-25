/*
 *  Cache-handling routined for MIPS CPUs
 *
 *  Copyright (c) 2003	Wolfgang Denk <wd@denx.de>
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
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/cacheops.h>

#define RA		t8

/*
 * 16kB is the maximum size of instruction and data caches on MIPS 4K,
 * 64kB is on 4KE, 24K, 5K, etc. Set bigger size for convenience.
 *
 * Note that the above size is the maximum size of primary cache. U-Boot
 * doesn't have L2 cache support for now.
 */
#define MIPS_MAX_CACHE_SIZE	0x10000

#define INDEX_BASE	KSEG0

	.macro	cache_op op addr
	.set	push
	.set	noreorder
	.set	mips3
	cache	\op, 0(\addr)
	.set	pop
	.endm

/*
 * cacheop macro to automate cache operations
 * first some helpers...
 */
#define _mincache(size, maxsize) \
   bltu  size,maxsize,9f ; \
   move  size,maxsize ;    \
9:

#define _align(minaddr, maxaddr, linesize) \
   .set noat ; \
   subu  AT,linesize,1 ;   \
   not   AT ;        \
   and   minaddr,AT ;      \
   addu  maxaddr,-1 ;      \
   and   maxaddr,AT ;      \
   .set at

/* general operations */
#define doop1(op1) \
   cache op1,0(a0)
#define doop2(op1, op2) \
   cache op1,0(a0) ;    \
   nop ;          \
   cache op2,0(a0)

/* specials for cache initialisation */
#define doop1lw(op1) \
   lw zero,0(a0)
#define doop1lw1(op1) \
   cache op1,0(a0) ;    \
   lw zero,0(a0) ;      \
   cache op1,0(a0)
#define doop121(op1,op2) \
   cache op1,0(a0) ;    \
   nop;           \
   cache op2,0(a0) ;    \
   nop;           \
   cache op1,0(a0)

#define _oploopn(minaddr, maxaddr, linesize, tag, ops) \
   .set  noreorder ;    \
10:   doop##tag##ops ;  \
   bne     minaddr,maxaddr,10b ; \
   add      minaddr,linesize ;   \
   .set  reorder

/* finally the cache operation macros */
#define vcacheopn(kva, n, cacheSize, cacheLineSize, tag, ops) \
   blez  n,11f ;        \
   addu  n,kva ;        \
   _align(kva, n, cacheLineSize) ; \
   _oploopn(kva, n, cacheLineSize, tag, ops) ; \
11:

#define icacheopn(kva, n, cacheSize, cacheLineSize, tag, ops) \
   _mincache(n, cacheSize);   \
   blez  n,11f ;        \
   addu  n,kva ;        \
   _align(kva, n, cacheLineSize) ; \
   _oploopn(kva, n, cacheLineSize, tag, ops) ; \
11:

#define vcacheop(kva, n, cacheSize, cacheLineSize, op) \
   vcacheopn(kva, n, cacheSize, cacheLineSize, 1, (op))

#define icacheop(kva, n, cacheSize, cacheLineSize, op) \
   icacheopn(kva, n, cacheSize, cacheLineSize, 1, (op))

	.macro	f_fill64 dst, offset, val
	LONG_S	\val, (\offset +  0 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  1 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  2 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  3 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  4 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  5 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  6 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  7 * LONGSIZE)(\dst)
#if LONGSIZE == 4
	LONG_S	\val, (\offset +  8 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset +  9 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 10 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 11 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 12 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 13 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 14 * LONGSIZE)(\dst)
	LONG_S	\val, (\offset + 15 * LONGSIZE)(\dst)
#endif
	.endm

/*
 * mips_init_icache(uint PRId, ulong icache_size, unchar icache_linesz)
 */
LEAF(mips_init_icache)
	blez	a1, 9f
	mtc0	zero, CP0_TAGLO
	/* clear tag to invalidate */
	PTR_LI		t0, INDEX_BASE
	PTR_ADDU	t1, t0, a1
1:	cache_op	Index_Store_Tag_I t0
	PTR_ADDU	t0, a2
	bne		t0, t1, 1b
	/* fill once, so data field parity is correct */
	PTR_LI		t0, INDEX_BASE
2:	cache_op	Fill t0
	PTR_ADDU	t0, a2
	bne		t0, t1, 2b
	/* invalidate again - prudent but not strictly neccessary */
	PTR_LI		t0, INDEX_BASE
1:	cache_op	Index_Store_Tag_I t0
	PTR_ADDU	t0, a2
	bne		t0, t1, 1b
9:	jr	ra
	END(mips_init_icache)

/*
 * mips_init_dcache(uint PRId, ulong dcache_size, unchar dcache_linesz)
 */
LEAF(mips_init_dcache)
	blez	a1, 9f
	mtc0	zero, CP0_TAGLO
	/* clear all tags */
	PTR_LI		t0, INDEX_BASE
	PTR_ADDU	t1, t0, a1
1:	cache_op	Index_Store_Tag_D t0
	PTR_ADDU	t0, a2
	bne		t0, t1, 1b
	/* load from each line (in cached space) */
	PTR_LI		t0, INDEX_BASE
2:	LONG_L		zero, 0(t0)
	PTR_ADDU	t0, a2
	bne		t0, t1, 2b
	/* clear all tags */
	PTR_LI		t0, INDEX_BASE
1:	cache_op	Index_Store_Tag_D t0
	PTR_ADDU	t0, a2
	bne		t0, t1, 1b
9:	jr	ra
	END(mips_init_dcache)

/*******************************************************************************
*
* mips_cache_reset - low level initialisation of the primary caches
*
* This routine initialises the primary caches to ensure that they
* have good parity.  It must be called by the ROM before any cached locations
* are used to prevent the possibility of data with bad parity being written to
* memory.
* To initialise the instruction cache it is essential that a source of data
* with good parity is available. This routine
* will initialise an area of memory starting at location zero to be used as
* a source of parity.
*
* RETURNS: N/A
*
*/
NESTED(mips_cache_reset, 0, ra)
	move	RA, ra
	li	t2, CFG_ICACHE_SIZE
	li	t3, CFG_DCACHE_SIZE
	li	t4, CFG_CACHELINE_SIZE
	move	t5, t4

	li	v0, MIPS_MAX_CACHE_SIZE

	/*
	 * Now clear that much memory starting from zero.
	 */
	PTR_LI		a0, KSEG1
	PTR_ADDU	a1, a0, v0
2:	PTR_ADDIU	a0, 64
	f_fill64	a0, -64, zero
	bne		a0, a1, 2b

	/*
	 * The caches are probably in an indeterminate state,
	 * so we force good parity into them by doing an
	 * invalidate, load/fill, invalidate for each line.
	 */

	/*
	 * Assume bottom of RAM will generate good parity for the cache.
	 */

	/*
	 * Initialize the I-cache first,
	 */
	move	a1, t2
	move	a2, t4
	bal	mips_init_icache

	/*
	 * then initialize D-cache.
	 */
	move	a1, t3
	move	a2, t5
	bal	mips_init_dcache

	jr	RA
	END(mips_cache_reset)

/*******************************************************************************
*
* dcache_status - get cache status
*
* RETURNS: 0 - cache disabled; 1 - cache enabled
*
*/
LEAF(dcache_status)
	mfc0	t0, CP0_CONFIG
	li	t1, CONF_CM_UNCACHED
	andi	t0, t0, CONF_CM_CMASK
	move	v0, zero
	beq	t0, t1, 2f
	li	v0, 1
2:	jr	ra
	END(dcache_status)

/*******************************************************************************
*
* dcache_disable - disable cache
*
* RETURNS: N/A
*
*/
LEAF(dcache_disable)
	mfc0	t0, CP0_CONFIG
	li	t1, -8
	and	t0, t0, t1
	ori	t0, t0, CONF_CM_UNCACHED
	mtc0	t0, CP0_CONFIG
	j	ra
	END(dcache_disable)

#ifdef CFG_INIT_RAM_LOCK_MIPS
/*******************************************************************************
*
* mips_cache_lock - lock RAM area pointed to by a0 in cache.
*
* RETURNS: N/A
*
*/
#if defined(CONFIG_PURPLE)
# define	CACHE_LOCK_SIZE	(CFG_DCACHE_SIZE/2)
#else
# define	CACHE_LOCK_SIZE	(CFG_DCACHE_SIZE)
#endif
	.globl	mips_cache_lock
	.ent	mips_cache_lock
mips_cache_lock:
	li	a1, K0BASE - CACHE_LOCK_SIZE
	addu	a0, a1
	li	a2, CACHE_LOCK_SIZE
	li	a3, CFG_CACHELINE_SIZE
	move	a1, a2
	icacheop(a0,a1,a2,a3,0x1d)

	j	ra

	.end	mips_cache_lock
#endif /* CFG_INIT_RAM_LOCK_MIPS */
