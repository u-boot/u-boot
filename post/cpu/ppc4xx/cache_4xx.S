/*
 * (C) Copyright 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Author: Igor Lisitsin <igor@emcraft.com>
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

#ifdef CONFIG_POST

#include <post.h>
#include <ppc_asm.tmpl>
#include <ppc_defs.h>
#include <asm/cache.h>
#include <asm/mmu.h>

#if CONFIG_POST & CFG_POST_CACHE

	.text

	/*
	 * All 44x variants deal with cache management differently
	 * because they have the address translation always enabled.
	 * The 40x ppc's don't use address translation in U-Boot at all,
	 * so we have to distinguish here between 40x and 44x.
	 */
#ifdef CONFIG_440
/* void cache_post_disable (int tlb)
 */
cache_post_disable:
	tlbre	r0, r3, 0x0002
	ori	r0, r0, TLB_WORD2_I_ENABLE@l
	tlbwe	r0, r3, 0x0002
	sync
	isync
	blr

/* void cache_post_wt (int tlb)
 */
cache_post_wt:
	tlbre	r0, r3, 0x0002
	ori	r0, r0, TLB_WORD2_W_ENABLE@l
	andi.	r0, r0, ~TLB_WORD2_I_ENABLE@l
	tlbwe	r0, r3, 0x0002
	sync
	isync
	blr

/* void cache_post_wb (int tlb)
 */
cache_post_wb:
	tlbre	r0, r3, 0x0002
	andi.	r0, r0, ~TLB_WORD2_W_ENABLE@l
	andi.	r0, r0, ~TLB_WORD2_I_ENABLE@l
	tlbwe	r0, r3, 0x0002
	sync
	isync
	blr
#else
/* void cache_post_disable (int tlb)
 */
cache_post_disable:
	lis	r0, 0x0000
	ori	r0, r0, 0x0000
	mtdccr	r0
	sync
	isync
	blr

/* void cache_post_wt (int tlb)
 */
cache_post_wt:
	lis	r0, 0x8000
	ori	r0, r0, 0x0000
	mtdccr	r0
	lis	r0, 0x8000
	ori	r0, r0, 0x0000
	mtdcwr	r0
	sync
	isync
	blr

/* void cache_post_wb (int tlb)
 */
cache_post_wb:
	lis	r0, 0x8000
	ori	r0, r0, 0x0000
	mtdccr	r0
	lis	r0, 0x0000
	ori	r0, r0, 0x0000
	mtdcwr	r0
	sync
	isync
	blr
#endif

/* void cache_post_dinvalidate (void *p, int size)
 */
cache_post_dinvalidate:
	dcbi	r0, r3
	addi	r3, r3, CFG_CACHELINE_SIZE
	subic.	r4, r4, CFG_CACHELINE_SIZE
	bgt	cache_post_dinvalidate
	sync
	blr

/* void cache_post_dstore (void *p, int size)
 */
cache_post_dstore:
	dcbst	r0, r3
	addi	r3, r3, CFG_CACHELINE_SIZE
	subic.	r4, r4, CFG_CACHELINE_SIZE
	bgt	cache_post_dstore
	sync
	blr

/* void cache_post_dtouch (void *p, int size)
 */
cache_post_dtouch:
	dcbt	r0, r3
	addi	r3, r3, CFG_CACHELINE_SIZE
	subic.	r4, r4, CFG_CACHELINE_SIZE
	bgt	cache_post_dtouch
	sync
	blr

/* void cache_post_iinvalidate (void)
 */
cache_post_iinvalidate:
	iccci	r0, r0
	sync
	blr

/* void cache_post_memset (void *p, int val, int size)
 */
cache_post_memset:
	mtctr	r5
1:
	stb	r4, 0(r3)
	addi	r3, r3, 1
	bdnz	1b
	blr

/* int cache_post_check (void *p, int size)
 */
cache_post_check:
	mtctr	r4
1:
	lbz	r0, 0(r3)
	addi	r3, r3, 1
	cmpwi	r0, 0xff
	bne	2f
	bdnz	1b
	li	r3, 0
	blr
2:
	li	r3, -1
	blr

#define CACHE_POST_DISABLE()		\
	mr	r3, r10;		\
	bl	cache_post_disable

#define CACHE_POST_WT()			\
	mr	r3, r10;		\
	bl	cache_post_wt

#define CACHE_POST_WB()			\
	mr	r3, r10;		\
	bl	cache_post_wb

#define CACHE_POST_DINVALIDATE()	\
	mr	r3, r11;		\
	mr	r4, r12;		\
	bl	cache_post_dinvalidate

#define CACHE_POST_DFLUSH()		\
	mr	r3, r11;		\
	mr	r4, r12;		\
	bl	cache_post_dflush

#define CACHE_POST_DSTORE()		\
	mr	r3, r11;		\
	mr	r4, r12;		\
	bl	cache_post_dstore

#define CACHE_POST_DTOUCH()		\
	mr	r3, r11;		\
	mr	r4, r12;		\
	bl	cache_post_dtouch

#define CACHE_POST_IINVALIDATE()	\
	bl	cache_post_iinvalidate

#define CACHE_POST_MEMSET(val)		\
	mr	r3, r11;		\
	li	r4, val;		\
	mr	r5, r12;		\
	bl	cache_post_memset

#define CACHE_POST_CHECK()		\
	mr	r3, r11;		\
	mr	r4, r12;		\
	bl	cache_post_check;	\
	mr	r13, r3

/*
 * Write and read 0xff pattern with caching enabled.
 */
	.global cache_post_test1
cache_post_test1:
	mflr	r9
	mr	r10, r3		/* tlb		*/
	mr	r11, r4		/* p		*/
	mr	r12, r5		/* size		*/

	CACHE_POST_WB()
	CACHE_POST_DINVALIDATE()

	/* Write the negative pattern to the test area */
	CACHE_POST_MEMSET(0xff)

	/* Read the test area */
	CACHE_POST_CHECK()

	CACHE_POST_DINVALIDATE()
	CACHE_POST_DISABLE()

	mr	r3, r13
	mtlr	r9
	blr

/*
 * Write zeroes with caching enabled.
 * Write 0xff pattern with caching disabled.
 * Read 0xff pattern with caching enabled.
 */
	.global cache_post_test2
cache_post_test2:
	mflr	r9
	mr	r10, r3		/* tlb		*/
	mr	r11, r4		/* p		*/
	mr	r12, r5		/* size		*/

	CACHE_POST_WB()
	CACHE_POST_DINVALIDATE()

	/* Write the zero pattern to the test area */
	CACHE_POST_MEMSET(0)

	CACHE_POST_DINVALIDATE()
	CACHE_POST_DISABLE()

	/* Write the negative pattern to the test area */
	CACHE_POST_MEMSET(0xff)

	CACHE_POST_WB()

	/* Read the test area */
	CACHE_POST_CHECK()

	CACHE_POST_DINVALIDATE()
	CACHE_POST_DISABLE()

	mr	r3, r13
	mtlr	r9
	blr

/*
 * Write-through mode test.
 * Write zeroes, store the cache, write 0xff pattern.
 * Invalidate the cache.
 * Check that 0xff pattern is read.
 */
	.global cache_post_test3
cache_post_test3:
	mflr	r9
	mr	r10, r3		/* tlb		*/
	mr	r11, r4		/* p		*/
	mr	r12, r5		/* size		*/

	CACHE_POST_WT()
	CACHE_POST_DINVALIDATE()

	/* Cache the test area */
	CACHE_POST_DTOUCH()

	/* Write the zero pattern to the test area */
	CACHE_POST_MEMSET(0)

	CACHE_POST_DSTORE()

	/* Write the negative pattern to the test area */
	CACHE_POST_MEMSET(0xff)

	CACHE_POST_DINVALIDATE()
	CACHE_POST_DISABLE()

	/* Read the test area */
	CACHE_POST_CHECK()

	mr	r3, r13
	mtlr	r9
	blr

/*
 * Write-back mode test.
 * Write 0xff pattern, store the cache, write zeroes.
 * Invalidate the cache.
 * Check that 0xff pattern is read.
 */
	.global cache_post_test4
cache_post_test4:
	mflr	r9
	mr	r10, r3		/* tlb		*/
	mr	r11, r4		/* p		*/
	mr	r12, r5		/* size		*/

	CACHE_POST_WB()
	CACHE_POST_DINVALIDATE()

	/* Cache the test area */
	CACHE_POST_DTOUCH()

	/* Write the negative pattern to the test area */
	CACHE_POST_MEMSET(0xff)

	CACHE_POST_DSTORE()

	/* Write the zero pattern to the test area */
	CACHE_POST_MEMSET(0)

	CACHE_POST_DINVALIDATE()
	CACHE_POST_DISABLE()

	/* Read the test area */
	CACHE_POST_CHECK()

	mr	r3, r13
	mtlr	r9
	blr

/*
 * Load the test instructions into the instruction cache.
 * Replace the test instructions.
 * Check that the original instructions are executed.
 */
	.global cache_post_test5
cache_post_test5:
	mflr	r9
	mr	r10, r3		/* tlb		*/
	mr	r11, r4		/* p		*/
	mr	r12, r5		/* size		*/

	CACHE_POST_WT()
	CACHE_POST_IINVALIDATE()

	/* Compute r13 = cache_post_test_inst */
	bl	cache_post_test5_reloc
cache_post_test5_reloc:
	mflr	r13
	lis	r0, (cache_post_test_inst - cache_post_test5_reloc)@h
	ori	r0, r0, (cache_post_test_inst - cache_post_test5_reloc)@l
	add	r13, r13, r0

	/* Copy the test instructions to the test area */
	lwz	r0, 0(r13)
	stw	r0, 0(r11)
	lwz	r0, 8(r13)
	stw	r0, 4(r11)
	sync

	/* Invalidate the cache line */
	icbi	r0, r11
	sync
	isync

	/* Execute the test instructions */
	mtlr	r11
	blrl

	/* Replace the test instruction */
	lwz	r0, 4(r13)
	stw	r0, 0(r11)
	sync

	/* Do not invalidate the cache line */
	isync

	/* Execute the test instructions */
	mtlr	r11
	blrl
	mr	r13, r3

	CACHE_POST_IINVALIDATE()
	CACHE_POST_DINVALIDATE()
	CACHE_POST_DISABLE()

	mr	r3, r13
	mtlr	r9
	blr

/*
 * Load the test instructions into the instruction cache.
 * Replace the test instructions and invalidate the cache.
 * Check that the replaced instructions are executed.
 */
	.global cache_post_test6
cache_post_test6:
	mflr	r9
	mr	r10, r3		/* tlb		*/
	mr	r11, r4		/* p		*/
	mr	r12, r5		/* size		*/

	CACHE_POST_WT()
	CACHE_POST_IINVALIDATE()

	/* Compute r13 = cache_post_test_inst */
	bl	cache_post_test6_reloc
cache_post_test6_reloc:
	mflr	r13
	lis	r0, (cache_post_test_inst - cache_post_test6_reloc)@h
	ori	r0, r0, (cache_post_test_inst - cache_post_test6_reloc)@l
	add	r13, r13, r0

	/* Copy the test instructions to the test area */
	lwz	r0, 4(r13)
	stw	r0, 0(r11)
	lwz	r0, 8(r13)
	stw	r0, 4(r11)
	sync

	/* Invalidate the cache line */
	icbi	r0, r11
	sync
	isync

	/* Execute the test instructions */
	mtlr	r11
	blrl

	/* Replace the test instruction */
	lwz	r0, 0(r13)
	stw	r0, 0(r11)
	sync

	/* Invalidate the cache line */
	icbi	r0, r11
	sync
	isync

	/* Execute the test instructions */
	mtlr	r11
	blrl
	mr	r13, r3

	CACHE_POST_IINVALIDATE()
	CACHE_POST_DINVALIDATE()
	CACHE_POST_DISABLE()

	mr	r3, r13
	mtlr	r9
	blr

/* Test instructions.
 */
cache_post_test_inst:
	li	r3, 0
	li	r3, -1
	blr

#endif /* CONFIG_POST & CFG_POST_CACHE */
#endif /* CONFIG_POST */
