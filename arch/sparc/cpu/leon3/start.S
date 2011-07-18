/* This is where the SPARC/LEON3 starts
 * Copyright (C) 2007,
 * Daniel Hellstrom, daniel@gaisler.com
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

#include <asm-offsets.h>
#include <config.h>
#include <asm/asmmacro.h>
#include <asm/winmacro.h>
#include <asm/psr.h>
#include <asm/stack.h>
#include <asm/leon.h>
#include <version.h>

/* Entry for traps which jump to a programmer-specified trap handler.  */
#define TRAPR(H)  \
	wr	%g0, 0xfe0, %psr; \
	mov	%g0, %tbr; \
	ba	(H); \
	mov	%g0, %wim;

#define TRAP(H) \
	mov	%psr, %l0; \
	ba	(H); \
	nop; nop;

#define TRAPI(ilevel) \
	mov	ilevel, %l7; \
	mov	%psr, %l0; \
	b	_irq_entry; \
	mov	%wim, %l3

/* Unexcpected trap will halt the processor by forcing it to error state */
#undef BAD_TRAP
#define BAD_TRAP ta 0; nop; nop; nop;

/* Software trap. Treat as BAD_TRAP for the time being... */
#define SOFT_TRAP TRAP(_hwerr)

#define PSR_INIT   0x1FC0	/* Disable traps, set s and ps */
#define WIM_INIT   2

/* All traps low-level code here must end with this macro. */
#define RESTORE_ALL b ret_trap_entry; clr %l6;

#define WRITE_PAUSE nop;nop;nop

WINDOWSIZE = (16 * 4)
ARGPUSHSIZE = (6 * 4)
ARGPUSH = (WINDOWSIZE + 4)
MINFRAME = (WINDOWSIZE + ARGPUSHSIZE + 4)

/* Number of register windows */
#ifndef CONFIG_SYS_SPARC_NWINDOWS
#error Must define number of SPARC register windows, default is 8
#endif

#define STACK_ALIGN	8
#define SA(X)	(((X)+(STACK_ALIGN-1)) & ~(STACK_ALIGN-1))

	.section ".start", "ax"
	.globl	_start, start, _trap_table
	.globl  _irq_entry, nmi_trap
	.globl  _reset_reloc

/* at address 0
 * Hardware traps
 */
start:
_start:
_trap_table:
	TRAPR(_hardreset);		! 00 reset trap
	BAD_TRAP;			! 01 instruction_access_exception
	BAD_TRAP;			! 02 illegal_instruction
	BAD_TRAP;			! 03 priveleged_instruction
	BAD_TRAP;			! 04 fp_disabled
	TRAP(_window_overflow);		! 05 window_overflow
	TRAP(_window_underflow);	! 06 window_underflow
	BAD_TRAP;			! 07 Memory Address Not Aligned
	BAD_TRAP;			! 08 Floating Point Exception
	BAD_TRAP;			! 09 Data Miss Exception
	BAD_TRAP;			! 0a Tagged Instruction Ovrflw
	BAD_TRAP;			! 0b Watchpoint Detected
	BAD_TRAP;			! 0c
	BAD_TRAP;			! 0d
	BAD_TRAP;			! 0e
	BAD_TRAP;			! 0f
	BAD_TRAP;			! 10
	TRAPI(1);			! 11 IRQ level 1
	TRAPI(2);			! 12 IRQ level 2
	TRAPI(3);			! 13 IRQ level 3
	TRAPI(4);			! 14 IRQ level 4
	TRAPI(5);			! 15 IRQ level 5
	TRAPI(6);			! 16 IRQ level 6
	TRAPI(7);			! 17 IRQ level 7
	TRAPI(8);			! 18 IRQ level 8
	TRAPI(9);			! 19 IRQ level 9
	TRAPI(10);			! 1a IRQ level 10
	TRAPI(11);			! 1b IRQ level 11
	TRAPI(12);			! 1c IRQ level 12
	TRAPI(13);			! 1d IRQ level 13
	TRAPI(14);			! 1e IRQ level 14
	TRAP(_nmi_trap);		! 1f IRQ level 15 /
					! NMI (non maskable interrupt)
	BAD_TRAP;			! 20 r_register_access_error
	BAD_TRAP;			! 21 instruction access error
	BAD_TRAP;			! 22
	BAD_TRAP;			! 23
	BAD_TRAP;			! 24 co-processor disabled
	BAD_TRAP;			! 25 uniplemented FLUSH
	BAD_TRAP;			! 26
	BAD_TRAP;			! 27
	BAD_TRAP;			! 28 co-processor exception
	BAD_TRAP;			! 29 data access error
	BAD_TRAP;			! 2a division by zero
	BAD_TRAP;			! 2b data store error
	BAD_TRAP;			! 2c data access MMU miss
	BAD_TRAP;			! 2d
	BAD_TRAP;			! 2e
	BAD_TRAP;			! 2f
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 30-33
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 34-37
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 38-3b
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 3c-3f
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 40-43
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 44-47
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 48-4b
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 4c-4f
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 50-53
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 54-57
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 58-5b
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 5c-5f

	/* implementaion dependent */
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 60-63
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 64-67
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 68-6b
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 6c-6f
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 70-73
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 74-77
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 78-7b
	BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP; ! 7c-7f

	/* Software traps, not handled */
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! 80-83
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! 84-87
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! 88-8b
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! 8c-8f
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! 90-93
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! 94-97
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! 98-9b
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! 9c-9f
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! a0-a3
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! a4-a7
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! a8-ab
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! ac-af
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! b0-b3
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! b4-b7
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! b8-bb
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! bc-bf
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! c0-c3
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! c4-c7
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! c8-cb
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! cc-cf
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! d0-d3
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! d4-d7
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! d8-db
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! dc-df
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! e0-e3
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! e4-e7
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! e8-eb
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! ec-ef
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! f0-f3
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! f4-f7
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! f8-fb
	SOFT_TRAP; SOFT_TRAP; SOFT_TRAP; SOFT_TRAP;	! fc-ff
/*
 * Version string
 */

	.data
	.extern leon3_snooping_avail
	.globl	version_string
version_string:
	.ascii U_BOOT_VERSION_STRING, "\0"

	.section	".text"
	.align 4

_hardreset:
1000:
	flush

	/* Enable I/D-Cache and Snooping */
	set	0x0081000f, %g2
	sta	%g2, [%g0] 2

	mov	%g0, %y
	clr	%g1
	clr	%g2
	clr	%g3
	clr	%g4
	clr	%g5
	clr	%g6
	clr	%g7

	mov	%asr17, %g3
	and	%g3, 0x1f, %g3
clear_window:
	mov	%g0, %l0
	mov	%g0, %l1
	mov	%g0, %l2
	mov	%g0, %l3
	mov	%g0, %l4
	mov	%g0, %l5
	mov	%g0, %l6
	mov	%g0, %l7
	mov	%g0, %o0
	mov	%g0, %o1
	mov	%g0, %o2
	mov	%g0, %o3
	mov	%g0, %o4
	mov	%g0, %o5
	mov	%g0, %o6
	mov	%g0, %o7
	subcc	%g3, 1, %g3
	bge	clear_window
	save

wininit:
	set	WIM_INIT, %g3
	mov	%g3, %wim

stackp:
	set	CONFIG_SYS_INIT_SP_OFFSET, %fp
	andn	%fp, 0x0f, %fp
	sub	%fp, 64, %sp

cpu_init_unreloc:
	call	cpu_init_f
	nop

/* un relocated start address of monitor */
#define TEXT_START _text

/* un relocated end address of monitor */
#define DATA_END __init_end

reloc:
	set	TEXT_START,%g2
	set	DATA_END,%g3
	set	CONFIG_SYS_RELOC_MONITOR_BASE,%g4
reloc_loop:
	ldd	[%g2],%l0
	ldd	[%g2+8],%l2
	std	%l0,[%g4]
	std	%l2,[%g4+8]
	inc	16,%g2
	subcc	%g3,%g2,%g0
	bne	reloc_loop
	inc	16,%g4

	clr	%l0
	clr	%l1
	clr	%l2
	clr	%l3
	clr	%g2

/* register g4 contain address to start
 * This means that BSS must be directly after data and code segments
 *
 * g3 is length of bss = (__bss_end-__bss_start)
 *
 */

clr_bss:
/* clear bss area (the relocated) */
	set	__bss_start,%g2
	set	__bss_end,%g3
	sub	%g3,%g2,%g3
	add	%g3,%g4,%g3
	clr	%g1	/* std %g0 uses g0 and g1 */
/* clearing 16byte a time ==> linker script need to align to 16 byte offset */
clr_bss_16:
	std	%g0,[%g4]
	std	%g0,[%g4+8]
	inc	16,%g4
	cmp	%g3,%g4
	bne	clr_bss_16
	nop

/* add offsets to GOT table */
fixup_got:
	set	__got_start,%g4
	set	__got_end,%g3
/*
 * new got offset = (old GOT-PTR (read with ld) -
 *   CONFIG_SYS_RELOC_MONITOR_BASE(from define) ) +
 *   Destination Address (from define)
 */
	set	CONFIG_SYS_RELOC_MONITOR_BASE,%g2
	set	TEXT_START, %g1
	add	%g4,%g2,%g4
	sub	%g4,%g1,%g4
	add	%g3,%g2,%g3
	sub	%g3,%g1,%g3
	sub	%g2,%g1,%g2	! prepare register with (new base address) -
				!  (old base address)
got_loop:
	ld	[%g4],%l0	! load old GOT-PTR
	add	%l0,%g2,%l0	! increase with (new base address) -
				!  (old base)
	st	%l0,[%g4]
	inc	4,%g4
	cmp	%g3,%g4
	bne	got_loop
	nop

prom_relocate:
	set	__prom_start, %g2
	set	__prom_end, %g3
	set	CONFIG_SYS_PROM_OFFSET, %g4

prom_relocate_loop:
	ldd	[%g2],%l0
	ldd	[%g2+8],%l2
	std	%l0,[%g4]
	std	%l2,[%g4+8]
	inc	16,%g2
	subcc	%g3,%g2,%g0
	bne	prom_relocate_loop
	inc	16,%g4

/* Trap table has been moved, lets tell CPU about
 * the new trap table address
 */

	set	CONFIG_SYS_RELOC_MONITOR_BASE, %g2
	wr	%g0, %g2, %tbr
	nop
	nop
	nop

/* If CACHE snooping is available in hardware the
 * variable leon3_snooping_avail will be set to
 * 0x800000 else 0.
 */
snoop_detect:
	sethi	%hi(0x00800000), %o0
	lda	[%g0] 2, %o1
	and	%o0, %o1, %o0
	sethi	%hi(leon3_snooping_avail+CONFIG_SYS_RELOC_MONITOR_BASE-CONFIG_SYS_TEXT_BASE), %o1
	st	%o0, [%lo(leon3_snooping_avail+CONFIG_SYS_RELOC_MONITOR_BASE-CONFIG_SYS_TEXT_BASE)+%o1]

/*	call	relocate*/
	nop
/* Call relocated init functions */
jump:
	set	cpu_init_f2,%o1
	set	CONFIG_SYS_RELOC_MONITOR_BASE,%o2
	add	%o1,%o2,%o1
	sub	%o1,%g1,%o1
	call	%o1
	clr	%o0

	set	board_init_f,%o1
	set	CONFIG_SYS_RELOC_MONITOR_BASE,%o2
	add	%o1,%o2,%o1
	sub	%o1,%g1,%o1
	call	%o1
	clr	%o0

dead:	ta 0				! if call returns...
	nop

/* Interrupt handler caller,
 * reg L7: interrupt number
 * reg L0: psr after interrupt
 * reg L1: PC
 * reg L2: next PC
 * reg L3: wim
 */
_irq_entry:
	SAVE_ALL

	or	%l0, PSR_PIL, %g2
	wr	%g2, 0x0, %psr
	WRITE_PAUSE
	wr	%g2, PSR_ET, %psr
	WRITE_PAUSE
	mov	%l7, %o0		! irq level
	set	handler_irq, %o1
	set	(CONFIG_SYS_RELOC_MONITOR_BASE-CONFIG_SYS_TEXT_BASE), %o2
	add	%o1, %o2, %o1
	call	%o1
	add	%sp, SF_REGS_SZ, %o1	! pt_regs ptr
	or	%l0, PSR_PIL, %g2	! restore PIL after handler_irq
	wr	%g2, PSR_ET, %psr	! keep ET up
	WRITE_PAUSE

	RESTORE_ALL

!Window overflow trap handler.
	.global _window_overflow

_window_overflow:

	mov	%wim, %l3		! Calculate next WIM
	mov	%g1, %l7
	srl	%l3, 1, %g1
	sll	%l3, (CONFIG_SYS_SPARC_NWINDOWS-1) , %l4
	or	%l4, %g1, %g1

	save				! Get into window to be saved.
	mov	%g1, %wim
	nop;
	nop;
	nop
	st	%l0, [%sp + 0];
	st	%l1, [%sp + 4];
	st	%l2, [%sp + 8];
	st	%l3, [%sp + 12];
	st	%l4, [%sp + 16];
	st	%l5, [%sp + 20];
	st	%l6, [%sp + 24];
	st	%l7, [%sp + 28];
	st	%i0, [%sp + 32];
	st	%i1, [%sp + 36];
	st	%i2, [%sp + 40];
	st	%i3, [%sp + 44];
	st	%i4, [%sp + 48];
	st	%i5, [%sp + 52];
	st	%i6, [%sp + 56];
	st	%i7, [%sp + 60];
	restore				! Go back to trap window.
	mov	%l7, %g1
	jmp	%l1			! Re-execute save.
	rett	%l2

/* Window underflow trap handler.  */

	.global  _window_underflow

_window_underflow:

	mov  %wim, %l3			! Calculate next WIM
	sll  %l3, 1, %l4
	srl  %l3, (CONFIG_SYS_SPARC_NWINDOWS-1), %l5
	or   %l5, %l4, %l5
	mov  %l5, %wim
	nop; nop; nop
	restore				! Two restores to get into the
	restore				! window to restore
	ld	[%sp + 0], %l0;		! Restore window from the stack
	ld	[%sp + 4], %l1;
	ld	[%sp + 8], %l2;
	ld	[%sp + 12], %l3;
	ld	[%sp + 16], %l4;
	ld	[%sp + 20], %l5;
	ld	[%sp + 24], %l6;
	ld	[%sp + 28], %l7;
	ld	[%sp + 32], %i0;
	ld	[%sp + 36], %i1;
	ld	[%sp + 40], %i2;
	ld	[%sp + 44], %i3;
	ld	[%sp + 48], %i4;
	ld	[%sp + 52], %i5;
	ld	[%sp + 56], %i6;
	ld	[%sp + 60], %i7;
	save				! Get back to the trap window.
	save
	jmp	%l1			! Re-execute restore.
	rett	%l2

	retl

_nmi_trap:
	nop
	jmp %l1
	rett %l2

_hwerr:
	ta 0
	nop
	nop
	b _hwerr			! loop infinite
	nop

/* Registers to not touch at all. */
#define t_psr      l0 /* Set by caller */
#define t_pc       l1 /* Set by caller */
#define t_npc      l2 /* Set by caller */
#define t_wim      l3 /* Set by caller */
#define t_twinmask l4 /* Set at beginning of this entry routine. */
#define t_kstack   l5 /* Set right before pt_regs frame is built */
#define t_retpc    l6 /* If you change this, change winmacro.h header file */
#define t_systable l7 /* Never touch this, could be the syscall table ptr. */
#define curptr     g6 /* Set after pt_regs frame is built */

trap_setup:
/* build a pt_regs trap frame. */
	sub	%fp, (SF_REGS_SZ + PT_REGS_SZ), %t_kstack
	PT_STORE_ALL(t_kstack, t_psr, t_pc, t_npc, g2)

	/* See if we are in the trap window. */
	mov	1, %t_twinmask
	sll	%t_twinmask, %t_psr, %t_twinmask ! t_twinmask = (1 << psr)
	andcc	%t_twinmask, %t_wim, %g0
	beq	1f		! in trap window, clean up
	nop

	/*-------------------------------------------------
	 * Spill , adjust %wim and go.
	 */
	srl	%t_wim, 0x1, %g2		! begin computation of new %wim

	set	(CONFIG_SYS_SPARC_NWINDOWS-1), %g3	!NWINDOWS-1

	sll	%t_wim, %g3, %t_wim	! NWINDOWS-1
	or	%t_wim, %g2, %g2
	and	%g2, 0xff, %g2

	save	%g0, %g0, %g0           ! get in window to be saved

	/* Set new %wim value */
	wr	%g2, 0x0, %wim

	/* Save the kernel window onto the corresponding stack. */
	RW_STORE(sp)

	restore	%g0, %g0, %g0
	/*-------------------------------------------------*/

1:
	/* Trap from kernel with a window available.
	 * Just do it...
	 */
	jmpl	%t_retpc + 0x8, %g0	! return to caller
	 mov	%t_kstack, %sp		! jump onto new stack

#define twin_tmp1 l4
#define glob_tmp  g4
#define curptr    g6
ret_trap_entry:
	wr	%t_psr, 0x0, %psr       ! enable nesting again, clear ET

	/* Will the rett land us in the invalid window? */
	mov	2, %g1
	sll	%g1, %t_psr, %g1

	set	CONFIG_SYS_SPARC_NWINDOWS, %g2	!NWINDOWS

	srl	%g1, %g2, %g2
	or	%g1, %g2, %g1
	rd	%wim, %g2
	andcc	%g2, %g1, %g0
	be	1f		! Nope, just return from the trap
	 sll	%g2, 0x1, %g1

	/* We have to grab a window before returning. */
	set	(CONFIG_SYS_SPARC_NWINDOWS-1), %g3	!NWINDOWS-1

	srl	%g2, %g3,  %g2
	or	%g1, %g2, %g1
	and	%g1, 0xff, %g1

	wr	%g1, 0x0, %wim

	/* Grrr, make sure we load from the right %sp... */
	PT_LOAD_ALL(sp, t_psr, t_pc, t_npc, g1)

	restore	%g0, %g0, %g0
	RW_LOAD(sp)
	b	2f
	save	%g0, %g0, %g0

	/* Reload the entire frame in case this is from a
	 * kernel system call or whatever...
	 */
1:
	PT_LOAD_ALL(sp, t_psr, t_pc, t_npc, g1)
2:
	wr	%t_psr, 0x0, %psr
	nop;
	nop;
	nop

	jmp	%t_pc
	rett	%t_npc

/* This is called from relocated C-code.
 * It resets the system by jumping to _start
 */
_reset_reloc:
	set	start, %l0
	call	%l0
	nop
