/*
 * U-boot - entry.h Routines for context saving and restoring
 *
 * Copyright (c) 2005 blackfin.uclinux.org
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

#ifndef __BLACKFIN_ENTRY_H
#define __BLACKFIN_ENTRY_H

#include <linux/config.h>
#include <asm/setup.h>
#include <asm/page.h>

/*
 * Stack layout in 'ret_from_exception':
 *
 */

/*
 * Register %p2 is now set to the current task throughout
 * the whole kernel.
 */

#ifdef __ASSEMBLY__

#define	LFLUSH_I_AND_D	0x00000808
#define	LSIGTRAP	5

/* process bits for task_struct.flags */
#define	PF_TRACESYS_OFF	3
#define	PF_TRACESYS_BIT	5
#define	PF_PTRACED_OFF	3
#define	PF_PTRACED_BIT	4
#define	PF_DTRACE_OFF	1
#define	PF_DTRACE_BIT	5

#define NEW_PT_REGS

#if defined(NEW_PT_REGS)

#define SAVE_ALL_INT		save_context_no_interrupts
#define SAVE_ALL_SYS		save_context_no_interrupts
#define SAVE_CONTEXT		save_context_with_interrupts

#define RESTORE_ALL		restore_context_no_interrupts
#define RESTORE_ALL_SYS		restore_context_no_interrupts
#define RESTORE_CONTEXT		restore_context_with_interrupts

#else

#define SAVE_ALL_INT		save_all_int
#define SAVE_ALL_SYS		save_all_sys
#define SAVE_CONTEXT		save_context
#define RESTORE_ALL		restore_context
#define RESTORE_CONTEXT		restore_context

#endif

/*
 * Code to save processor context.
 * We even save the register which are preserved by a function call
 * - r4, r5, r6, r7, p3, p4, p5
 */
.macro save_context_with_interrupts
	[--sp] = R0;
	[--sp] = ( R7:0, P5:0 );
	[--sp] = fp;
	[--sp] = usp;

	[--sp] = i0;
	[--sp] = i1;
	[--sp] = i2;
	[--sp] = i3;

	[--sp] = m0;
	[--sp] = m1;
	[--sp] = m2;
	[--sp] = m3;

	[--sp] = l0;
	[--sp] = l1;
	[--sp] = l2;
	[--sp] = l3;

	[--sp] = b0;
	[--sp] = b1;
	[--sp] = b2;
	[--sp] = b3;
	[--sp] = a0.x;
	[--sp] = a0.w;
	[--sp] = a1.x;
	[--sp] = a1.w;

	[--sp] = LC0;
	[--sp] = LC1;
	[--sp] = LT0;
	[--sp] = LT1;
	[--sp] = LB0;
	[--sp] = LB1;

	[--sp] = ASTAT;

	[--sp] = r0;	/* Skip reserved */
	[--sp] = RETS;
	[--sp] = RETI;
	[--sp] = RETX;
	[--sp] = RETN;
	[--sp] = RETE;
	[--sp] = SEQSTAT;
	[--sp] = SYSCFG;
	[--sp] = r0;	/* Skip IPEND as well. */
.endm

.macro save_context_no_interrupts
	[--sp] = R0;
	[--sp] = ( R7:0, P5:0 );
	[--sp] = fp;
	[--sp] = usp;

	[--sp] = i0;
	[--sp] = i1;
	[--sp] = i2;
	[--sp] = i3;

	[--sp] = m0;
	[--sp] = m1;
	[--sp] = m2;
	[--sp] = m3;

	[--sp] = l0;
	[--sp] = l1;
	[--sp] = l2;
	[--sp] = l3;

	[--sp] = b0;
	[--sp] = b1;
	[--sp] = b2;
	[--sp] = b3;
	[--sp] = a0.x;
	[--sp] = a0.w;
	[--sp] = a1.x;
	[--sp] = a1.w;

	[--sp] = LC0;
	[--sp] = LC1;
	[--sp] = LT0;
	[--sp] = LT1;
	[--sp] = LB0;
	[--sp] = LB1;

	[--sp] = ASTAT;

	[--sp] = r0;	/* Skip reserved */
	[--sp] = RETS;
	r0 = RETI;
	[--sp] = r0;
	[--sp] = RETX;
	[--sp] = RETN;
	[--sp] = RETE;
	[--sp] = SEQSTAT;
	[--sp] = SYSCFG;
	[--sp] = r0;	/* Skip IPEND as well. */
.endm

.macro restore_context_no_interrupts
	sp += 4;
	SYSCFG = [sp++];
	SEQSTAT = [sp++];
	RETE = [sp++];
	RETN = [sp++];
	RETX = [sp++];
	r0 = [sp++];
	RETI = r0;
	RETS = [sp++];

	sp += 4;

	ASTAT = [sp++];

	LB1 = [sp++];
	LB0 = [sp++];
	LT1 = [sp++];
	LT0 = [sp++];
	LC1 = [sp++];
	LC0 = [sp++];

	a1.w = [sp++];
	a1.x = [sp++];
	a0.w = [sp++];
	a0.x = [sp++];
	b3 = [sp++];
	b2 = [sp++];
	b1 = [sp++];
	b0 = [sp++];

	l3 = [sp++];
	l2 = [sp++];
	l1 = [sp++];
	l0 = [sp++];

	m3 = [sp++];
	m2 = [sp++];
	m1 = [sp++];
	m0 = [sp++];

	i3 = [sp++];
	i2 = [sp++];
	i1 = [sp++];
	i0 = [sp++];

	sp += 4;
	fp = [sp++];

	( R7 : 0, P5 : 0) = [ SP ++ ];
	sp += 4;
.endm

.macro restore_context_with_interrupts
	sp += 4;
	SYSCFG = [sp++];
	SEQSTAT = [sp++];
	RETE = [sp++];
	RETN = [sp++];
	RETX = [sp++];
	RETI = [sp++];
	RETS = [sp++];

	sp += 4;

	ASTAT = [sp++];

	LB1 = [sp++];
	LB0 = [sp++];
	LT1 = [sp++];
	LT0 = [sp++];
	LC1 = [sp++];
	LC0 = [sp++];

	a1.w = [sp++];
	a1.x = [sp++];
	a0.w = [sp++];
	a0.x = [sp++];
	b3 = [sp++];
	b2 = [sp++];
	b1 = [sp++];
	b0 = [sp++];

	l3 = [sp++];
	l2 = [sp++];
	l1 = [sp++];
	l0 = [sp++];

	m3 = [sp++];
	m2 = [sp++];
	m1 = [sp++];
	m0 = [sp++];

	i3 = [sp++];
	i2 = [sp++];
	i1 = [sp++];
	i0 = [sp++];

	sp += 4;
	fp = [sp++];

	( R7 : 0, P5 : 0) = [ SP ++ ];
	sp += 4;
.endm

#if !defined(NEW_PT_REGS)
/*
 * a -1 in the orig_r0 field signifies
 * that the stack frame is NOT for syscall
 */
.macro	save_all_int
/* reserved and disable the single step of SYSCFG, by Steven Chen 03/07/10 */
	[--sp] = r0;
	r0.l = 0x30;		/* Errata for BF533 */
	r0.h = 0x0;
	syscfg = r0;		/* disable single step flag in SYSCFG */
	r0 = [sp++];
	[--sp] = syscfg;	/* store SYSCFG */

	[--sp] = r0;	/* Reserved for IPEND */
	[--sp] = fp;
	[--sp] = usp;
	[--sp] = r0;

	[--sp] = r0;
	r0 = [sp + 8];
	[--sp] = a0.x;
	[--sp] = a1.x;
	[--sp] = a0.w;
	[--sp] = a1.w;
	[--sp] = rets;
	[--sp] = astat;
	[--sp] = seqstat;
	[--sp] = retx;	/* current pc when exception happens */
	[--sp] = ( r7:5, p5:0 );
	[--sp] = r1;
	[--sp] = r2;
	[--sp] = r4;
	[--sp] = r3;
.endm

.macro	save_all_sys
	[--sp] = r0;
	[--sp] = r0;
	[--sp] = a0.x;
	[--sp] = a1.x;
	[--sp] = a0.w;
	[--sp] = a1.w;
	[--sp] = rets;
	[--sp] = astat;
	[--sp] = seqstat;
	[--sp] = retx;	/* current pc when exception happens */
	[--sp] = ( r7:5, p5:0 );
	[--sp] = r1;
	[--sp] = r2;
	[--sp] = r4;
	[--sp] = r3;
.endm

.macro	restore_all
	r3 = [sp++];
	r4 = [sp++];
	r2 = [sp++];
	r1 = [sp++];
	( r7:5, p5:0 ) = [sp++];
	retx = [sp++];
	seqstat = [sp++];
	astat = [sp++];
	rets = [sp++];
	a1.w = [sp++];
	a0.w = [sp++];
	a1.x = [sp++];
	a0.x = [sp++];
	sp += 4;	/* orig r0 */
	r0 = [sp++];

	sp += 4;
	fp = [sp++];
	sp +=4;		/* Skip the IPEND */

	syscfg = [sp++];

.endm

#endif

#define STR(X) 			STR1(X)
#define STR1(X) 		#X

#if defined(NEW_PT_REGS)

#define PT_OFF_ORIG_R0		208
#define PT_OFF_SR		8

#else

#define PT_OFF_ORIG_R0		0x54
#define PT_OFF_SR		0x38	/* seqstat in pt_regs */

#endif
#endif

#endif
