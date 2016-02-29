/*
 * Copyright (C) 2012-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <linux/linkage.h>
#include <linux/sizes.h>
#include <asm/system.h>

#include "ssc-regs.h"

ENTRY(lowlevel_init)
	mov	r8, lr			@ persevere link reg across call

	/*
	 * The UniPhier Boot ROM loads SPL code to the L2 cache.
	 * But CPUs can only do instruction fetch now because start.S has
	 * cleared C and M bits.
	 * First we need to turn on MMU and Dcache again to get back
	 * data access to L2.
	 */
	mrc	p15, 0, r0, c1, c0, 0	@ SCTLR (System Control Register)
	orr	r0, r0, #(CR_C | CR_M)	@ enable MMU and Dcache
	mcr	p15, 0, r0, c1, c0, 0

#ifdef CONFIG_DEBUG_LL
	bl	debug_ll_init
#endif

	bl	setup_init_ram		@ RAM area for stack and page talbe

	/*
	 * Now we are using the page table embedded in the Boot ROM.
	 * It is not handy since it is not a straight mapped table for sLD3.
	 * Also, the access to the external bus is prohibited.  What we need
	 * to do next is to create a page table and switch over to it.
	 */
	bl	create_page_table
	bl	v7_flush_dcache_all

	/* Disable MMU and Dcache before switching Page Table */
	mrc	p15, 0, r0, c1, c0, 0	@ SCTLR (System Control Register)
	bic	r0, r0, #(CR_C | CR_M)	@ disable MMU and Dcache
	mcr	p15, 0, r0, c1, c0, 0

	bl	enable_mmu

	mov	lr, r8			@ restore link
	mov	pc, lr			@ back to my caller
ENDPROC(lowlevel_init)

ENTRY(enable_mmu)
	mrc	p15, 0, r0, c2, c0, 2	@ TTBCR (Translation Table Base Control Register)
	bic	r0, r0, #0x37
	orr	r0, r0, #0x20		@ disable TTBR1
	mcr	p15, 0, r0, c2, c0, 2

	orr	r0, r12, #0x8		@ Outer Cacheability for table walks: WBWA
	mcr	p15, 0, r0, c2, c0, 0   @ TTBR0

	mov	r0, #0
	mcr	p15, 0, r0, c8, c7, 0	@ invalidate TLBs

	mov	r0, #-1			@ manager for all domains (No permission check)
	mcr	p15, 0, r0, c3, c0, 0   @ DACR (Domain Access Control Register)

	dsb
	isb
	/*
	 * MMU on:
	 * TLBs was already invalidated in "../start.S"
	 * So, we don't need to invalidate it here.
	 */
	mrc	p15, 0, r0, c1, c0, 0	@ SCTLR (System Control Register)
	orr	r0, r0, #(CR_C | CR_M)	@ MMU and Dcache enable
	mcr	p15, 0, r0, c1, c0, 0

	mov	pc, lr
ENDPROC(enable_mmu)

/*
 * For PH1-Pro4 or older SoCs, the size of WAY is 32KB.
 * It is large enough for tmp RAM.
 */
#define BOOT_RAM_SIZE	(SZ_32K)
#define BOOT_RAM_BASE	((CONFIG_SPL_STACK) - (BOOT_RAM_SIZE))
#define BOOT_WAY_BITS	(0x00000100)   /* way 8 */

ENTRY(setup_init_ram)
	/*
	 * Touch to zero for the boot way
	 */
0:
	/*
	 * set SSCOQM, SSCOQAD, SSCOQSZ, SSCOQWN in this order
	 */
	ldr	r0, = 0x00408006	@ touch to zero with address range
	ldr	r1, = SSCOQM
	str	r0, [r1]
	ldr	r0, = BOOT_RAM_BASE
	ldr	r1, = SSCOQAD
	str	r0, [r1]
	ldr	r0, = BOOT_RAM_SIZE
	ldr	r1, = SSCOQSZ
	str	r0, [r1]
	ldr	r0, = BOOT_WAY_BITS
	ldr	r1, = SSCOQWN
	str	r0, [r1]
	ldr	r1, = SSCOPPQSEF
	ldr	r0, [r1]
	cmp	r0, #0			@ check if the command is successfully set
	bne	0b			@ try again if an error occurs

	ldr	r1, = SSCOLPQS
1:
	ldr	r0, [r1]
	cmp	r0, #0x4
	bne	1b			@ wait until the operation is completed
	str	r0, [r1]		@ clear the complete notification flag

	mov	pc, lr
ENDPROC(setup_init_ram)

#define DEVICE	0x00002002 /* Non-shareable Device */
#define NORMAL	0x0000000e /* Normal Memory Write-Back, No Write-Allocate */

ENTRY(create_page_table)
	ldr	r0, = DEVICE
	ldr	r1, = BOOT_RAM_BASE
	mov	r12, r1			@ r12 is preserved during D-cache flush
0:	str	r0, [r1], #4		@ specify all the sections as Device
	adds	r0, r0, #0x00100000
	bcc	0b

	ldr	r0, = NORMAL
	str	r0, [r12]		@ mark the first section as Normal
	add	r0, r0, #0x00100000
	str	r0, [r12, #4]		@ mark the second section as Normal
	mov	pc, lr
ENDPROC(create_page_table)

/* We don't use Thumb instructions for now */
#define ARM(x...)	x
#define THUMB(x...)

/*
 *	v7_flush_dcache_all()
 *
 *	Flush the whole D-cache.
 *
 *	Corrupted registers: r0-r7, r9-r11 (r6 only in Thumb mode)
 *
 *	- mm    - mm_struct describing address space
 *
 *	Note: copied from arch/arm/mm/cache-v7.S of Linux 4.4
 */
ENTRY(v7_flush_dcache_all)
	dmb					@ ensure ordering with previous memory accesses
	mrc	p15, 1, r0, c0, c0, 1		@ read clidr
	mov	r3, r0, lsr #23			@ move LoC into position
	ands	r3, r3, #7 << 1			@ extract LoC*2 from clidr
	beq	finished			@ if loc is 0, then no need to clean
start_flush_levels:
	mov	r10, #0				@ start clean at cache level 0
flush_levels:
	add	r2, r10, r10, lsr #1		@ work out 3x current cache level
	mov	r1, r0, lsr r2			@ extract cache type bits from clidr
	and	r1, r1, #7			@ mask of the bits for current cache only
	cmp	r1, #2				@ see what cache we have at this level
	blt	skip				@ skip if no cache, or just i-cache
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	isb					@ isb to sych the new cssr&csidr
	mrc	p15, 1, r1, c0, c0, 0		@ read the new csidr
	and	r2, r1, #7			@ extract the length of the cache lines
	add	r2, r2, #4			@ add 4 (line length offset)
	movw	r4, #0x3ff
	ands	r4, r4, r1, lsr #3		@ find maximum number on the way size
	clz	r5, r4				@ find bit position of way size increment
	movw	r7, #0x7fff
	ands	r7, r7, r1, lsr #13		@ extract max number of the index size
loop1:
	mov	r9, r7				@ create working copy of max index
loop2:
 ARM(	orr	r11, r10, r4, lsl r5	)	@ factor way and cache number into r11
 THUMB(	lsl	r6, r4, r5		)
 THUMB(	orr	r11, r10, r6		)	@ factor way and cache number into r11
 ARM(	orr	r11, r11, r9, lsl r2	)	@ factor index number into r11
 THUMB(	lsl	r6, r9, r2		)
 THUMB(	orr	r11, r11, r6		)	@ factor index number into r11
	mcr	p15, 0, r11, c7, c14, 2		@ clean & invalidate by set/way
	subs	r9, r9, #1			@ decrement the index
	bge	loop2
	subs	r4, r4, #1			@ decrement the way
	bge	loop1
skip:
	add	r10, r10, #2			@ increment cache number
	cmp	r3, r10
	bgt	flush_levels
finished:
	mov	r10, #0				@ swith back to cache level 0
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	dsb	st
	isb
	mov	pc, lr
ENDPROC(v7_flush_dcache_all)
