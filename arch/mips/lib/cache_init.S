/*
 *  Cache-handling routined for MIPS CPUs
 *
 *  Copyright (c) 2003	Wolfgang Denk <wd@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/cacheops.h>

#ifndef CONFIG_SYS_MIPS_CACHE_MODE
#define CONFIG_SYS_MIPS_CACHE_MODE CONF_CM_CACHABLE_NONCOHERENT
#endif

#ifdef CONFIG_64BIT
# define RA		ta3
#else
# define RA		t7
#endif

#define INDEX_BASE	CKSEG0

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
	blez		a1, 9f
	mtc0		zero, CP0_TAGLO
	/* clear tag to invalidate */
	PTR_LI		t0, INDEX_BASE
	PTR_ADDU	t1, t0, a1
1:	cache		INDEX_STORE_TAG_I, 0(t0)
	PTR_ADDU	t0, a2
	bne		t0, t1, 1b
	/* fill once, so data field parity is correct */
	PTR_LI		t0, INDEX_BASE
2:	cache		FILL, 0(t0)
	PTR_ADDU	t0, a2
	bne		t0, t1, 2b
	/* invalidate again - prudent but not strictly neccessary */
	PTR_LI		t0, INDEX_BASE
1:	cache		INDEX_STORE_TAG_I, 0(t0)
	PTR_ADDU	t0, a2
	bne		t0, t1, 1b
9:	jr		ra
	END(mips_init_icache)

/*
 * mips_init_dcache(uint PRId, ulong dcache_size, unchar dcache_linesz)
 */
LEAF(mips_init_dcache)
	blez		a1, 9f
	mtc0		zero, CP0_TAGLO
	/* clear all tags */
	PTR_LI		t0, INDEX_BASE
	PTR_ADDU	t1, t0, a1
1:	cache		INDEX_STORE_TAG_D, 0(t0)
	PTR_ADDU	t0, a2
	bne		t0, t1, 1b
	/* load from each line (in cached space) */
	PTR_LI		t0, INDEX_BASE
2:	LONG_L		zero, 0(t0)
	PTR_ADDU	t0, a2
	bne		t0, t1, 2b
	/* clear all tags */
	PTR_LI		t0, INDEX_BASE
1:	cache		INDEX_STORE_TAG_D, 0(t0)
	PTR_ADDU	t0, a2
	bne		t0, t1, 1b
9:	jr		ra
	END(mips_init_dcache)

	.macro	l1_info		sz, line_sz, off
	.set	push
	.set	noat

	mfc0	$1, CP0_CONFIG, 1

	/* detect line size */
	srl	\line_sz, $1, \off + MIPS_CONF1_DL_SHIFT - MIPS_CONF1_DA_SHIFT
	andi	\line_sz, \line_sz, (MIPS_CONF1_DL >> MIPS_CONF1_DL_SHIFT)
	move	\sz, zero
	beqz	\line_sz, 10f
	li	\sz, 2
	sllv	\line_sz, \sz, \line_sz

	/* detect associativity */
	srl	\sz, $1, \off + MIPS_CONF1_DA_SHIFT - MIPS_CONF1_DA_SHIFT
	andi	\sz, \sz, (MIPS_CONF1_DA >> MIPS_CONF1_DA_SHIFT)
	addi	\sz, \sz, 1

	/* sz *= line_sz */
	mul	\sz, \sz, \line_sz

	/* detect log32(sets) */
	srl	$1, $1, \off + MIPS_CONF1_DS_SHIFT - MIPS_CONF1_DA_SHIFT
	andi	$1, $1, (MIPS_CONF1_DS >> MIPS_CONF1_DS_SHIFT)
	addiu	$1, $1, 1
	andi	$1, $1, 0x7

	/* sz <<= log32(sets) */
	sllv	\sz, \sz, $1

	/* sz *= 32 */
	li	$1, 32
	mul	\sz, \sz, $1
10:
	.set	pop
	.endm
/*
 * mips_cache_reset - low level initialisation of the primary caches
 *
 * This routine initialises the primary caches to ensure that they have good
 * parity.  It must be called by the ROM before any cached locations are used
 * to prevent the possibility of data with bad parity being written to memory.
 *
 * To initialise the instruction cache it is essential that a source of data
 * with good parity is available. This routine will initialise an area of
 * memory starting at location zero to be used as a source of parity.
 *
 * RETURNS: N/A
 *
 */
NESTED(mips_cache_reset, 0, ra)
	move	RA, ra

#ifdef CONFIG_SYS_ICACHE_SIZE
	li	t2, CONFIG_SYS_ICACHE_SIZE
	li	t8, CONFIG_SYS_CACHELINE_SIZE
#else
	l1_info	t2, t8, MIPS_CONF1_IA_SHIFT
#endif

#ifdef CONFIG_SYS_DCACHE_SIZE
	li	t3, CONFIG_SYS_DCACHE_SIZE
	li	t9, CONFIG_SYS_CACHELINE_SIZE
#else
	l1_info	t3, t9, MIPS_CONF1_DA_SHIFT
#endif

	/* Determine the largest L1 cache size */
#if defined(CONFIG_SYS_ICACHE_SIZE) && defined(CONFIG_SYS_DCACHE_SIZE)
#if CONFIG_SYS_ICACHE_SIZE > CONFIG_SYS_DCACHE_SIZE
	li	v0, CONFIG_SYS_ICACHE_SIZE
#else
	li	v0, CONFIG_SYS_DCACHE_SIZE
#endif
#else
	move	v0, t2
	sltu	t1, t2, t3
	movn	v0, t3, t1
#endif
	/*
	 * Now clear that much memory starting from zero.
	 */
	PTR_LI		a0, CKSEG1
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
	move	a2, t8
	PTR_LA	v1, mips_init_icache
	jalr	v1

	/*
	 * then initialize D-cache.
	 */
	move	a1, t3
	move	a2, t9
	PTR_LA	v1, mips_init_dcache
	jalr	v1

	jr	RA
	END(mips_cache_reset)

/*
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

/*
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
	jr	ra
	END(dcache_disable)

/*
 * dcache_enable - enable cache
 *
 * RETURNS: N/A
 *
 */
LEAF(dcache_enable)
	mfc0	t0, CP0_CONFIG
	ori	t0, CONF_CM_CMASK
	xori	t0, CONF_CM_CMASK
	ori	t0, CONFIG_SYS_MIPS_CACHE_MODE
	mtc0	t0, CP0_CONFIG
	jr	ra
	END(dcache_enable)
