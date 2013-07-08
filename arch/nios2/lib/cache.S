/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>

	.text

	.global flush_dcache

flush_dcache:
	add	r5, r5, r4
	movhi	r8, %hi(CONFIG_SYS_DCACHELINE_SIZE)
	ori	r8, r8, %lo(CONFIG_SYS_DCACHELINE_SIZE)
0:	flushd	0(r4)
	add	r4, r4, r8
	bltu	r4, r5, 0b
	ret


	.global flush_icache

flush_icache:
	add	r5, r5, r4
	movhi	r8, %hi(CONFIG_SYS_ICACHELINE_SIZE)
	ori	r8, r8, %lo(CONFIG_SYS_ICACHELINE_SIZE)
1:	flushi	r4
	add	r4, r4, r8
	bltu	r4, r5, 1b
	ret

	.global flush_dcache_range

flush_dcache_range:
	movhi	r8, %hi(CONFIG_SYS_DCACHELINE_SIZE)
	ori	r8, r8, %lo(CONFIG_SYS_DCACHELINE_SIZE)
0:	flushd	0(r4)
	add	r4, r4, r8
	bltu	r4, r5, 0b
	ret

	.global flush_cache

flush_cache:
	add	r5, r5, r4
	mov	r9, r4
	mov	r10, r5

	movhi	r8, %hi(CONFIG_SYS_DCACHELINE_SIZE)
	ori	r8, r8, %lo(CONFIG_SYS_DCACHELINE_SIZE)
0:	flushd	0(r4)
	add	r4, r4, r8
	bltu	r4, r5, 0b

	mov	r4, r9
	mov	r5, r10
	movhi	r8, %hi(CONFIG_SYS_ICACHELINE_SIZE)
	ori	r8, r8, %lo(CONFIG_SYS_ICACHELINE_SIZE)
1:	flushi	r4
	add	r4, r4, r8
	bltu	r4, r5, 1b

	sync
	flushp
	ret
