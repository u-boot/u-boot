/*
 *  Copyright (C) 1998	Dan Malek <dmalek@jlc.net>
 *  Copyright (C) 1999	Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *  Copyright (C) 2000-2003 Wolfgang Denk <wd@denx.de>
 *  Coldfire contribution by Bernhard Kuhn <bkuhn@metrowerks.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include "version.h"

#ifndef	 CONFIG_IDENT_STRING
#define	 CONFIG_IDENT_STRING ""
#endif

#define MCF_MBAR 0x10000000
#define MEM_BUILTIN_ADDR 0x20000000
#define MEM_BUILTIN_SIZE 0x1000
#define DRAM_ADDR 0x0
#define DRAM_SIZE 0x400000

	.text

	.globl	_start
_start:
	nop
	nop
	move.w #0x2700,%sr

	move.l	#0, %d0
	movec	%d0, %VBR

#ifdef CONFIG_M5272
	move.l	#(MCF_MBAR+1), %d0
	move.c	%d0, %MBAR

	move.l	#(MEM_BUILTIN_ADDR+1), %d0
	movec	%d0, %RAMBAR0

	move.l	#0x01000000, %d0		/* Invalidate cache cmd */
	movec	%d0, %CACR			/* Invalidate cache */
	move.l	#0x0000c000, %d0		/* Setup cache mask */
	movec	%d0, %ACR0			/* Enable cache */
	move.l	#0xff00c000, %d0		/* Setup cache mask */
	movec	%d0, %ACR1			/* Enable cache */
	move.l	#0x80000100, %d0		/* Setup cache mask */
	movec	%d0, %CACR			/* Enable cache */
#endif

	move.l	#_sbss,%a0
	move.l	#_ebss,%d0
1:
	clr.l	(%a0)+
	cmp.l	%a0,%d0
	bne.s	1b

/*	move.l	#MEM_BUILTIN_ADDR+MEM_BUILTIN_SIZE, %sp */
	move.l	#DRAM_ADDR+DRAM_SIZE, %sp
	clr.l %sp@-

	jsr board_init_f

	.globl	exception_handler
exception_handler:
	move.w #0x2700,%sr
	lea %sp@(-60),%sp
	movem.l %d0-%d7/%a0-%a6,%sp@
	jsr do_exception
	movem.l %sp@,%d0-%d7/%a0-%a6
	lea %sp@(60),%sp
	rte

	.globl	buserror_handler
buserror_handler:
	move.w #0x2700,%sr
	lea %sp@(-60),%sp
	movem.l %d0-%d7/%a0-%a6,%sp@
	jsr do_buserror
	movem.l %sp@,%d0-%d7/%a0-%a6
	lea %sp@(60),%sp
	rte

	.globl	addresserror_handler
addresserror_handler:
	move.w #0x2700,%sr
	lea %sp@(-60),%sp
	movem.l %d0-%d7/%a0-%a6,%sp@
	jsr do_buserror
	movem.l %sp@,%d0-%d7/%a0-%a6
	lea %sp@(60),%sp
	rte

	.globl	get_endaddr
get_endaddr:
	movel #_end,%d0
	rts

#ifdef CONFIG_M5272
	.globl	icache_enable
icache_enable:
	move.l	#0x01000000, %d0		/* Invalidate cache cmd */
	movec	%d0, %CACR			/* Invalidate cache */
	move.l	#0x0000c000, %d0		/* Setup cache mask */
	movec	%d0, %ACR0			/* Enable cache */
	move.l	#0xff00c000, %d0		/* Setup cache mask */
	movec	%d0, %ACR1			/* Enable cache */
	move.l	#0x80000100, %d0		/* Setup cache mask */
	movec	%d0, %CACR			/* Enable cache */
	moveq	#1, %d0
	move.l	%d0, icache_state
	rts

	.globl	icache_disable
icache_disable:
	move.l	#0x00000100, %d0		/* Setup cache mask */
	movec	%d0, %CACR			/* Enable cache */
	clr.l	%d0				/* Setup cache mask */
	movec	%d0, %ACR0			/* Enable cache */
	movec	%d0, %ACR1			/* Enable cache */
	moveq	#0, %d0
	move.l	%d0, icache_state
	rts
#endif

#ifdef CONFIG_M5282
	.globl	icache_enable
icache_enable:
	rts

	.globl	icache_disable
icache_disable:
	rts
#endif

	.globl	icache_status
icache_status:
	move.l	icache_state, %d0
	rts

	.data
icache_state:
	.long	1

	.globl	version_string
version_string:
	.ascii U_BOOT_VERSION
	.ascii " (", __DATE__, " - ", __TIME__, ")"
	.ascii CONFIG_IDENT_STRING, "\0"
