/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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
