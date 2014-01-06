/*
 *  Copyright (C) 2000	Murray Jensen <Murray.Jensen@cmst.csiro.au>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <command.h>
#include <asm/ppc4xx.h>
#include <version.h>

#define CONFIG_405GP 1		/* needed for Linux kernel header files */

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
	/* icache */
	iccci   r0,r0		/* iccci invalidates the entire I cache */
	/* dcache */
	addi    r6,0,0x0000     /* clear GPR 6 */
	addi    r7,r0, 128	/* do loop for # of dcache lines */
				/* NOTE: dccci invalidates both */
	mtctr   r7              /* ways in the D cache */
..dcloop:
	dccci   0,r6            /* invalidate line */
	addi    r6,r6, 32	/* bump to next line */
	bdnz    ..dcloop
	blr

	.globl	kgdb_flush_cache_range
kgdb_flush_cache_range:
	li	r5,L1_CACHE_BYTES-1
	andc	r3,r3,r5
	subf	r4,r3,r4
	add	r4,r4,r5
	srwi.	r4,r4,L1_CACHE_SHIFT
	beqlr
	mtctr	r4
	mr	r6,r3
1:	dcbst	0,r3
	addi	r3,r3,L1_CACHE_BYTES
	bdnz	1b
	sync			/* wait for dcbst's to get to ram */
	mtctr	r4
2:	icbi	0,r6
	addi	r6,r6,L1_CACHE_BYTES
	bdnz	2b
	SYNC
	blr

#endif
