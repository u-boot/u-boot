/*
 * Copyright (C) 2007,2008 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 * Copyright (C) 2008 Renesas Solutions Corp.

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

#include <asm-offsets.h>
#include <config.h>
#include <version.h>

	.text
	.align	2

	.global	_start
_sh_start:
	.long 0x00000010	/* Ppower ON reset PC*/
	.long 0x00000000
	.long 0x00000010	/* Manual reset PC */
	.long 0x00000000
_init:
	mov.l	._lowlevel_init, r0
100:	bsrf	r0
	nop
	bsr	1f
	nop
1:	sts	pr, r5
	mov.l	._reloc_dst, r4
	add	#(_sh_start-1b), r5
	mov.l	._reloc_dst_end, r6

2:	mov.l	@r5+, r1
	mov.l	r1, @r4
	add	#4, r4
	cmp/hs	r6, r4
	bf	2b

	mov.l	._bss_start, r4
	mov.l	._bss_end, r5
	mov	#0, r1

3:	mov.l	r1, @r4			/* bss clear */
	add	#4, r4
	cmp/hs	r5, r4
	bf	3b

	mov.l	._gd_init, r13		/* global data */
	mov.l	._stack_init, r15	/* stack */

	mov.l	._sh_generic_init, r0
	jsr	@r0
	nop

loop:
	bra	loop

	.align	2

._lowlevel_init:	.long	(lowlevel_init - (100b + 4))
._reloc_dst:		.long	reloc_dst
._reloc_dst_end:	.long	reloc_dst_end
._bss_start:		.long	bss_start
._bss_end:		.long	bss_end
._gd_init:		.long	(_sh_start - GENERATED_GBL_DATA_SIZE)
._stack_init:	.long	(_sh_start - GENERATED_GBL_DATA_SIZE - CONFIG_SYS_MALLOC_LEN - 16)
._sh_generic_init:	.long	sh_generic_init
