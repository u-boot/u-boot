/*
 *  Copyright (C) 2000	Murray Jensen <Murray.Jensen@cmst.csiro.au>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <command.h>
#include <mpc8260.h>
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
	mfspr	r3, HID0
	ori	r3, r3, HID0_ICFI|HID0_DCI	/* Invalidate All */
	SYNC
	mtspr	HID0, r3
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
