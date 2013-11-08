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

#define RA		t9

#define INDEX_BASE	CKSEG0

	.macro	cache_op op addr
	.set	push
	.set	noreorder
	.set	mips3
	cache	\op, 0(\addr)
	.set	pop
	.endm

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
1:	cache_op	INDEX_STORE_TAG_I t0
	PTR_ADDU	t0, a2
	bne		t0, t1, 1b
	/* fill once, so data field parity is correct */
	PTR_LI		t0, INDEX_BASE
2:	cache_op	FILL t0
	PTR_ADDU	t0, a2
	bne		t0, t1, 2b
	/* invalidate again - prudent but not strictly neccessary */
	PTR_LI		t0, INDEX_BASE
1:	cache_op	INDEX_STORE_TAG_I t0
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
1:	cache_op	INDEX_STORE_TAG_D t0
	PTR_ADDU	t0, a2
	bne		t0, t1, 1b
	/* load from each line (in cached space) */
	PTR_LI		t0, INDEX_BASE
2:	LONG_L		zero, 0(t0)
	PTR_ADDU	t0, a2
	bne		t0, t1, 2b
	/* clear all tags */
	PTR_LI		t0, INDEX_BASE
1:	cache_op	INDEX_STORE_TAG_D t0
	PTR_ADDU	t0, a2
	bne		t0, t1, 1b
9:	jr		ra
	END(mips_init_dcache)

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

#if !defined(CONFIG_SYS_ICACHE_SIZE) || !defined(CONFIG_SYS_DCACHE_SIZE) || \
    !defined(CONFIG_SYS_CACHELINE_SIZE)
	/* read Config1 for use below */
	mfc0	t5, CP0_CONFIG, 1
#endif

#ifdef CONFIG_SYS_CACHELINE_SIZE
	li	t7, CONFIG_SYS_CACHELINE_SIZE
	li	t8, CONFIG_SYS_CACHELINE_SIZE
#else
	/* Detect I-cache line size. */
	srl	t8, t5, MIPS_CONF1_IL_SHIFT
	andi	t8, t8, (MIPS_CONF1_IL >> MIPS_CONF1_IL_SHIFT)
	beqz	t8, 1f
	li	t6, 2
	sllv	t8, t6, t8

1:	/* Detect D-cache line size. */
	srl	t7, t5, MIPS_CONF1_DL_SHIFT
	andi	t7, t7, (MIPS_CONF1_DL >> MIPS_CONF1_DL_SHIFT)
	beqz	t7, 1f
	li	t6, 2
	sllv	t7, t6, t7
1:
#endif

#ifdef CONFIG_SYS_ICACHE_SIZE
	li	t2, CONFIG_SYS_ICACHE_SIZE
#else
	/* Detect I-cache size. */
	srl	t6, t5, MIPS_CONF1_IS_SHIFT
	andi	t6, t6, (MIPS_CONF1_IS >> MIPS_CONF1_IS_SHIFT)
	li	t4, 32
	xori	t2, t6, 0x7
	beqz	t2, 1f
	addi	t6, t6, 1
	sllv	t4, t4, t6
1:	/* At this point t4 == I-cache sets. */
	mul	t2, t4, t8
	srl	t6, t5, MIPS_CONF1_IA_SHIFT
	andi	t6, t6, (MIPS_CONF1_IA >> MIPS_CONF1_IA_SHIFT)
	addi	t6, t6, 1
	/* At this point t6 == I-cache ways. */
	mul	t2, t2, t6
#endif

#ifdef CONFIG_SYS_DCACHE_SIZE
	li	t3, CONFIG_SYS_DCACHE_SIZE
#else
	/* Detect D-cache size. */
	srl	t6, t5, MIPS_CONF1_DS_SHIFT
	andi	t6, t6, (MIPS_CONF1_DS >> MIPS_CONF1_DS_SHIFT)
	li	t4, 32
	xori	t3, t6, 0x7
	beqz	t3, 1f
	addi	t6, t6, 1
	sllv	t4, t4, t6
1:	/* At this point t4 == I-cache sets. */
	mul	t3, t4, t7
	srl	t6, t5, MIPS_CONF1_DA_SHIFT
	andi	t6, t6, (MIPS_CONF1_DA >> MIPS_CONF1_DA_SHIFT)
	addi	t6, t6, 1
	/* At this point t6 == I-cache ways. */
	mul	t3, t3, t6
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
	move	a2, t7
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
