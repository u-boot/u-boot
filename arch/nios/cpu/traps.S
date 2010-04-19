/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
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

/*************************************************************************
 * Register window underflow
 *
 * The register window underflow exception occurs whenever the lowest
 * valid register window is in use (CWP=LO_LIMIT) and a save instruction
 * is issued. The save moves CWP below LO_LIMIT, %sp is set as normal,
 * then the exception is generated prior to executing the instruction
 * after the save.
 ************************************************************************/
	.text
	.global _cwp_lolimit
	.align	4

_cwp_lolimit:

	/* Sixteen words are always allocated by the compiler in every
	 * procedure's stack frame, always starting at %sp, for saving
	 * 'in' and 'local' registers on a window overflow.
	 *
	 * Save the 'global' and 'in' regs on stack. They are restored
	 * at cwp = HI_LIMIT. The 'local' regs aren't in-use at this point.
	 */
	sts	[%sp,0], %g0		/* Save 'global' regs*/
	sts	[%sp,1], %g1
	sts	[%sp,2], %g2
	sts	[%sp,3], %g3
	sts	[%sp,4], %g4
	sts	[%sp,5], %g5
	sts	[%sp,6], %g6
	sts	[%sp,7], %g7

	sts	[%sp,8], %i0		/* Save 'in' regs */
	sts	[%sp,9], %i1
	sts	[%sp,10], %i2
	sts	[%sp,11], %i3
	sts	[%sp,12], %i4
	sts	[%sp,13], %i5
	sts	[%sp,14], %i6
	sts	[%sp,15], %i7

	/* Save current %sp and return address in a global so they are
	 * available at cwp = HI_LIMIT ... where the 'global'/'in' regs
	 * are restored. NOTE: %sp changes with cwp.
	 */
	mov	%g7, %o7
	mov	%g6, %sp

	/* Get LO_LIMIT/HI_LIMIT to know where to start & stop. Note: in
	 * the underflow exception, cwp is __NOT__ guaranteed to be zero.
	 * If the OCI debug module is enabled the reset value for LO_LIMIT
	 * is 2, not 1 -- so cwp can be 1 or 0.
	 */
	pfx	2			/* WVALID */
	rdctl	%g1
	mov	%g2, %g1
	pfx	0
	and	%g1, 0x1f		/* g1 <- LO_LIMIT */
	lsri	%g2, 5
	pfx	0
	and	%g2,0x1f		/* g2 <- HI_LIMIT */

	/* Set istatus so cwp = HI_LIMIT after tret
	 */
	movi	%g5, 0x1f
	lsli	%g5, 4
	not	%g5			/* mask to clr cwp */
	pfx	1			/* istatus */
	rdctl	%g0
	and	%g0, %g5		/* clear cwp field */

	mov	%g4, %g2
	lsli	%g4, 4
	or	%g0, %g4		/* cwp = HI_LIMIT */
	pfx	1
	wrctl	%g0			/* update istatus */

	/* Now move up the register file, saving as we go. When loop
	 * is first entered, %g1 is at LO_LIMIT.
	 */
0:
	restore				/* cwp++ */
	sts	[%sp,0], %l0		/* Save "local" regs*/
	sts	[%sp,1], %l1
	sts	[%sp,2], %l2
	sts	[%sp,3], %l3
	sts	[%sp,4], %l4
	sts	[%sp,5], %l5
	sts	[%sp,6], %l6
	sts	[%sp,7], %l7

	sts	[%sp,8], %i0		/* Save 'in' regs */
	sts	[%sp,9], %i1
	sts	[%sp,10], %i2
	sts	[%sp,11], %i3
	sts	[%sp,12], %i4
	sts	[%sp,13], %i5
	sts	[%sp,14], %i6
	sts	[%sp,15], %i7

	cmp	%g1, %g2		/* cwp == HI_LIMIT ? */
	skps	cc_ne			/* if so, we're done */
	br	1f
	nop				/* delay slot */

	inc	%g1			/* g1 <- cwp++ */
	br	0b
	nop				/* delay slot */

	/* At this point cwp = HI_LIMIT, so the global/in regs that were
	 * in place when the underflow occurred must be restored using
	 * the original stack pointer (saved in g6).
	 */
1:
	mov	%o7, %g7		/* restore return addr */
	mov	%sp, %g6		/* Restore original sp */

	lds	%g0, [%sp,0]		/* Restore 'global' regs*/
	lds	%g1, [%sp,1]
	lds	%g2, [%sp,2]
	lds	%g3, [%sp,3]
	lds	%g4, [%sp,4]
	lds	%g5, [%sp,5]
	lds	%g6, [%sp,6]
	lds	%g7, [%sp,7]

	lds	%i0, [%sp,8]		/* Restore 'in' regs*/
	lds	%i1, [%sp,9]
	lds	%i2, [%sp,10]
	lds	%i3, [%sp,11]
	lds	%i4, [%sp,12]
	lds	%i5, [%sp,13]
	lds	%i6, [%sp,14]
	lds	%i7, [%sp,15]

	tret	%o7			/* All done */

/*************************************************************************
 * Register window overflow
 *
 * The register window overflow exception occurs whenever the highest
 * valid register window is in use (cwp = HI_LIMIT) and a restore
 * instruction is issued. Control is transferred to the overflow handler
 * before the instruction following restore is executed.
 *
 * When a register window overflow exception is taken, the exception
 * handler sees cwp at HI_LIMIT.
 ************************************************************************/
	.text
	.global _cwp_hilimit
	.align	4

_cwp_hilimit:

	/* Save 'global'/'in' regs on the stack -- will restore when cwp
	 * is at LO_LIMIT. Locals don't need saving as they are going away.
	 */
	sts	[%sp,0], %g0		/* Save "global" regs*/
	sts	[%sp,1], %g1
	sts	[%sp,2], %g2
	sts	[%sp,3], %g3
	sts	[%sp,4], %g4
	sts	[%sp,5], %g5
	sts	[%sp,6], %g6
	sts	[%sp,7], %g7

	sts	[%sp,8], %i0		/* Save 'in' regs */
	sts	[%sp,9], %i1
	sts	[%sp,10], %i2
	sts	[%sp,11], %i3
	sts	[%sp,12], %i4
	sts	[%sp,13], %i5
	sts	[%sp,14], %i6
	sts	[%sp,15], %i7

	/* The current %sp must be available in global to restore regs
	 * saved on stack. Need return addr as well ;-)
	 */
	mov	%g7, %o7
	mov	%g6, %sp

	/* Get HI_LIMIT & LO_LIMIT
	 */
	pfx	2			/* WVALID */
	rdctl	%g1
	mov	%g2, %g1
	pfx	0
	and	%g1, 0x1f		/* g1 <- LO_LIMIT */
	lsri	%g2, 5
	pfx	0
	and	%g2,0x1f		/* g2 <- HI_LIMIT */

	/* Set istatus so cwp = LO_LIMIT after tret
	 */
	movi	%g5, 0x1f
	lsli	%g5, 4
	not	%g5			/* mask to clr cwp */
	pfx	1			/* istatus */
	rdctl	%g0
	and	%g0, %g5		/* clear cwp field */

	mov	%g4, %g1		/* g4 <- LO_LIMIT */
	lsli	%g4, 4
	or	%g0, %g4		/* cwp = LO_LIMIT */
	pfx	1
	wrctl	%g0			/* update istatus */

	/* Move to cwp = LO_LIMIT-1 and restore 'in' regs.
	 */
	subi	%g4,(1 << 4)		/* g4 <- LO_LIMIT - 1 */
	rdctl	%g0
	and	%g0, %g5		/* clear cwp field */
	or	%g0, %g4		/* cwp = LO_LIMIT - 1 */
	wrctl	%g0			/* update status */
	nop

	mov	%sp, %g6		/* Restore sp */
	lds	%i0, [%sp,8]		/* Restore 'in' regs */
	lds	%i1, [%sp,9]
	lds	%i2, [%sp,10]
	lds	%i3, [%sp,11]
	lds	%i4, [%sp,12]
	lds	%i5, [%sp,13]
	lds	%i6, [%sp,14]		/* sp in next window */
	lds	%i7, [%sp,15]

	/* Starting at LO_LIMIT-1, move up the register file, restoring
	 * along the way.
	 */
0:
	restore				/* cwp++ */
	lds	%l0, [%sp,0]		/* Restore 'local' regs*/
	lds	%l1, [%sp,1]
	lds	%l2, [%sp,2]
	lds	%l3, [%sp,3]
	lds	%l4, [%sp,4]
	lds	%l5, [%sp,5]
	lds	%l6, [%sp,6]
	lds	%l7, [%sp,7]

	lds	%i0, [%sp,8]		/* Restore 'in' regs */
	lds	%i1, [%sp,9]
	lds	%i2, [%sp,10]
	lds	%i3, [%sp,11]
	lds	%i4, [%sp,12]
	lds	%i5, [%sp,13]
	lds	%i6, [%sp,14]		/* sp in next window */
	lds	%i7, [%sp,15]

	cmp	%g1, %g2		/* cwp == HI_LIMIT ? */
	skps	cc_ne			/* if so, we're done */
	br	1f
	nop				/* delay slot */

	inc	%g1			/* cwp++ */
	br	0b
	nop				/* delay slot */

	/* All windows have been updated at this point, but the globals
	 * still need to be restored. Go to cwp = LO_LIMIT-1 to get
	 * some registers to use.
	 */
1:
	rdctl	%g0
	and	%g0, %g5		/* clear cwp field */
	or	%g0, %g4		/* cwp = LO_LIMIT - 1 */
	wrctl	%g0			/* update status */
	nop

	/* Now there are some registers available to use in restoring
	 * the globals.
	 */
	mov	%sp, %g6
	mov	%o7, %g7

	lds	%g0, [%sp,0]		/* Restore "global" regs*/
	lds	%g1, [%sp,1]
	lds	%g2, [%sp,2]
	lds	%g3, [%sp,3]
	lds	%g4, [%sp,4]
	lds	%g5, [%sp,5]
	lds	%g6, [%sp,6]
	lds	%g7, [%sp,7]

	/* The tret moves istatus -> status. istatus was already set for
	 * cwp = LO_LIMIT.
	 */

	tret	%o7			/* done */

/*************************************************************************
 * Default exception handler
 *
 * The default handler passes control to external_interrupt(). So trap
 * or hardware interrupt hanlders can be installed using the familiar
 * irq_install_handler().
 *
 * Here, the stack is fixed-up and cwp is incremented prior to calling
 * external_interrupt(). This lets the underflow and overflow handlers
 * operate normally during the exception.
 ************************************************************************/
	.text
	.global _def_xhandler
	.align	4

_def_xhandler:

	/* Allocate some stack space: 16 words at %sp to accomodate
	 * a reg window underflow, 8 words to save interrupted task's
	 * 'out' regs (which are now the 'in' regs), 8 words to preserve
	 * the 'global' regs and 3 words to save the return address,
	 * status and istatus. istatus must be saved in the event an
	 * underflow occurs in a dispatched handler. status is saved so
	 * a handler can access it on stack.
	 */
	pfx	%hi((16+16+3) * 4)
	subi	%fp, %lo((16+16+3) * 4)
	mov	%sp, %fp

	/* Save the 'global' regs and the interrupted task's 'out' regs
	 * (our 'in' regs) along with the return addr, status & istatus.
	 * First 16 words are for underflow exception.
	 */
	rdctl	%l0			/* status */
	pfx	1			/* istatus */
	rdctl	%l1

	sts	[%sp,16+0], %g0		/* Save 'global' regs*/
	sts	[%sp,16+1], %g1
	sts	[%sp,16+2], %g2
	sts	[%sp,16+3], %g3
	sts	[%sp,16+4], %g4
	sts	[%sp,16+5], %g5
	sts	[%sp,16+6], %g6
	sts	[%sp,16+7], %g7

	sts	[%sp,16+8], %i0		/* Save 'in' regs */
	sts	[%sp,16+9], %i1
	sts	[%sp,16+10], %i2
	sts	[%sp,16+11], %i3
	sts	[%sp,16+12], %i4
	sts	[%sp,16+13], %i5
	sts	[%sp,16+14], %i6
	sts	[%sp,16+15], %i7

	sts	[%sp,16+16], %l0	/* status */
	sts	[%sp,16+17], %l1	/* istatus */
	sts	[%sp,16+18], %o7	/* return addr */

	/* Move to cwp+1 ... this guarantees cwp is at or above LO_LIMIT.
	 * Need to set IPRI=3 and IE=1 to enable underflow exceptions.
	 * NOTE: only the 'out' regs have been saved ... can't touch
	 * the 'in' or 'local' here.
	 */
	restore				/* cwp++ */
	rdctl	%o0			/* o0 <- status */

	pfx	%hi(0x7e00)
	movi	%o1, %lo(0x7e00)
	not	%o1
	and	%o0, %o1		/* clear IPRI */

	pfx	%hi(0x8600)
	movi	%o1, %lo(0x8600)
	or	%o0, %o1		/* IPRI=3, IE=1 */

	wrctl	%o0			/* o0 -> status */
	nop

	/* It's ok to call a C routine now since cwp >= LO_LIMIT,
	 * interrupt task's registers are/will be preserved, and
	 * underflow exceptions can be handled.
	 */
	pfx	%hi(external_interrupt@h)
	movi	%o1, %lo(external_interrupt@h)
	pfx	%xhi(external_interrupt@h)
	movhi	%o1, %xlo(external_interrupt@h)
	bgen	%o0, 4+2		/* 16 * 4 */
	add	%o0, %sp		/* Ptr to regs */
	call	%o1
	nop

	/* Move back to the exception register window, restore the 'out'
	 * registers, then return from exception.
	 */
	rdctl	%o0			/* o0 <- status */
	subi	%o0, 16
	wrctl	%o0			/* cwp-- */
	nop

	mov	%sp, %fp
	lds	%g0, [%sp,16+0]		/* Restore 'global' regs*/
	lds	%g1, [%sp,16+1]
	lds	%g2, [%sp,16+2]
	lds	%g3, [%sp,16+3]
	lds	%g4, [%sp,16+4]
	lds	%g5, [%sp,16+5]
	lds	%g6, [%sp,16+6]
	lds	%g7, [%sp,16+7]

	lds	%i0, [%sp,16+8]		/* Restore 'in' regs*/
	lds	%i1, [%sp,16+9]
	lds	%i2, [%sp,16+10]
	lds	%i3, [%sp,16+11]
	lds	%i4, [%sp,16+12]
	lds	%i5, [%sp,16+13]
	lds	%i6, [%sp,16+14]
	lds	%i7, [%sp,16+15]

	lds	%l0, [%sp,16+16]	/* status */
	lds	%l1, [%sp,16+17]	/* istatus */
	lds	%o7, [%sp,16+18]	/* return addr */

	pfx	1
	wrctl	%l1			/* restore istatus */

	pfx	%hi((16+16+3) * 4)
	addi	%sp, %lo((16+16+3) * 4)
	mov	%fp, %sp

	tret	%o7			/* Done */


/*************************************************************************
 * Timebase Timer Interrupt -- This has identical structure to above,
 * but calls timer_interrupt().  Doing it this way keeps things similar
 * to other architectures (e.g. ppc).
 ************************************************************************/
	.text
	.global _timebase_int
	.align	4

_timebase_int:

	/* Allocate  stack space.
	 */
	pfx	%hi((16+16+3) * 4)
	subi	%fp, %lo((16+16+3) * 4)
	mov	%sp, %fp

	/* Save the 'global' regs & 'out' regs (our 'in' regs)
	 */
	rdctl	%l0			/* status */
	pfx	1			/* istatus */
	rdctl	%l1

	sts	[%sp,16+0], %g0		/* Save 'global' regs*/
	sts	[%sp,16+1], %g1
	sts	[%sp,16+2], %g2
	sts	[%sp,16+3], %g3
	sts	[%sp,16+4], %g4
	sts	[%sp,16+5], %g5
	sts	[%sp,16+6], %g6
	sts	[%sp,16+7], %g7

	sts	[%sp,16+8], %i0		/* Save 'in' regs */
	sts	[%sp,16+9], %i1
	sts	[%sp,16+10], %i2
	sts	[%sp,16+11], %i3
	sts	[%sp,16+12], %i4
	sts	[%sp,16+13], %i5
	sts	[%sp,16+14], %i6
	sts	[%sp,16+15], %i7

	sts	[%sp,16+16], %l0	/* status */
	sts	[%sp,16+17], %l1	/* istatus */
	sts	[%sp,16+18], %o7	/* return addr */

	/* Move to cwp+1.
	 */
	restore				/* cwp++ */
	rdctl	%o0			/* o0 <- status */

	pfx	%hi(0x7e00)
	movi	%o1, %lo(0x7e00)
	not	%o1
	and	%o0, %o1		/* clear IPRI */

	pfx	%hi(0x8600)
	movi	%o1, %lo(0x8600)
	or	%o0, %o1		/* IPRI=3, IE=1 */

	wrctl	%o0			/* o0 -> status */
	nop

	/* Call timer_interrupt()
	 */
	pfx	%hi(timer_interrupt@h)
	movi	%o1, %lo(timer_interrupt@h)
	pfx	%xhi(timer_interrupt@h)
	movhi	%o1, %xlo(timer_interrupt@h)
	bgen	%o0, 4+2		/* 16 * 4 */
	add	%o0, %sp		/* Ptr to regs */
	call	%o1
	nop

	/* Move back to the exception register window, restore the 'out'
	 * registers, then return from exception.
	 */
	rdctl	%o0			/* o0 <- status */
	subi	%o0, 16
	wrctl	%o0			/* cwp-- */
	nop

	mov	%sp, %fp
	lds	%g0, [%sp,16+0]		/* Restore 'global' regs*/
	lds	%g1, [%sp,16+1]
	lds	%g2, [%sp,16+2]
	lds	%g3, [%sp,16+3]
	lds	%g4, [%sp,16+4]
	lds	%g5, [%sp,16+5]
	lds	%g6, [%sp,16+6]
	lds	%g7, [%sp,16+7]

	lds	%i0, [%sp,16+8]		/* Restore 'in' regs*/
	lds	%i1, [%sp,16+9]
	lds	%i2, [%sp,16+10]
	lds	%i3, [%sp,16+11]
	lds	%i4, [%sp,16+12]
	lds	%i5, [%sp,16+13]
	lds	%i6, [%sp,16+14]
	lds	%i7, [%sp,16+15]

	lds	%l0, [%sp,16+16]	/* status */
	lds	%l1, [%sp,16+17]	/* istatus */
	lds	%o7, [%sp,16+18]	/* return addr */

	pfx	1
	wrctl	%l1			/* restore istatus */

	pfx	%hi((16+16+3) * 4)
	addi	%sp, %lo((16+16+3) * 4)
	mov	%fp, %sp

	tret	%o7			/* Done */

/*************************************************************************
 * GDB stubs
 ************************************************************************/
	.text
	.global _brkpt_hw_int, _brkpt_sw_int
	.align	4

_brkpt_hw_int:
	movi	%l1, 9
	pfx	3
	wrctl	%l1
	pfx	4
	wrctl	%l1

_brkpt_sw_int:
	movi	%l1, 9
	pfx	3
	wrctl	%l1
	pfx	4
	wrctl	%l1

	tret	%o7
