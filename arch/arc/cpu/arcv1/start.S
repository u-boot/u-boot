/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/arcregs.h>

/*
 * Note on the LD/ST addressing modes with address register write-back
 *
 * LD.a same as LD.aw
 *
 * LD.a    reg1, [reg2, x]  => Pre Incr
 *      Eff Addr for load = [reg2 + x]
 *
 * LD.ab   reg1, [reg2, x]  => Post Incr
 *      Eff Addr for load = [reg2]
 */

.macro PUSH reg
	st.a	\reg, [%sp, -4]
.endm

.macro PUSHAX aux
	lr	%r9, [\aux]
	PUSH	%r9
.endm

.macro  SAVE_R1_TO_R24
	PUSH	%r1
	PUSH	%r2
	PUSH	%r3
	PUSH	%r4
	PUSH	%r5
	PUSH	%r6
	PUSH	%r7
	PUSH	%r8
	PUSH	%r9
	PUSH	%r10
	PUSH	%r11
	PUSH	%r12
	PUSH	%r13
	PUSH	%r14
	PUSH	%r15
	PUSH	%r16
	PUSH	%r17
	PUSH	%r18
	PUSH	%r19
	PUSH	%r20
	PUSH	%r21
	PUSH	%r22
	PUSH	%r23
	PUSH	%r24
.endm

.macro SAVE_ALL_SYS
	/* saving %r0 to reg->r0 in advance since we read %ecr into it */
	st	%r0, [%sp, -8]
	lr	%r0, [%ecr]	/* all stack addressing is manual so far */
	st	%r0, [%sp]
	st	%sp, [%sp, -4]
	/* now move %sp to reg->r0 position so we can do "push" automatically */
	sub	%sp, %sp, 8

	SAVE_R1_TO_R24
	PUSH	%r25
	PUSH	%gp
	PUSH	%fp
	PUSH	%blink
	PUSHAX	%eret
	PUSHAX	%erstatus
	PUSH	%lp_count
	PUSHAX	%lp_end
	PUSHAX	%lp_start
	PUSHAX	%erbta
.endm

.macro SAVE_EXCEPTION_SOURCE
#ifdef CONFIG_MMU
	/* If MMU exists exception faulting address is loaded in EFA reg */
	lr	%r0, [%efa]
#else
	/* Otherwise in ERET (exception return) reg */
	lr	%r0, [%eret]
#endif
.endm

.section .ivt, "ax",@progbits
.align 4
_ivt:
	/* Critical system events */
	j	_start			/* 0 - 0x000 */
	j	memory_error		/* 1 - 0x008 */
	j	instruction_error	/* 2 - 0x010 */

	/* Device interrupts */
.rept	29
	j	interrupt_handler	/* 3:31 - 0x018:0xF8 */
.endr
	/* Exceptions */
	j	EV_MachineCheck		/* 0x100, Fatal Machine check  (0x20) */
	j	EV_TLBMissI		/* 0x108, Intruction TLB miss  (0x21) */
	j	EV_TLBMissD		/* 0x110, Data TLB miss        (0x22) */
	j	EV_TLBProtV		/* 0x118, Protection Violation (0x23)
							or Misaligned Access  */
	j	EV_PrivilegeV		/* 0x120, Privilege Violation  (0x24) */
	j	EV_Trap			/* 0x128, Trap exception       (0x25) */
	j	EV_Extension		/* 0x130, Extn Intruction Excp (0x26) */

.text
.globl _start
_start:
	/* Setup interrupt vector base that matches "__text_start" */
	sr	__ivt_start, [ARC_AUX_INTR_VEC_BASE]

	/* Setup stack pointer */
	mov	%sp, CONFIG_SYS_INIT_SP_ADDR
	mov	%fp, %sp

	/* Clear bss */
	mov	%r0, __bss_start
	mov	%r1, __bss_end

clear_bss:
	st.ab	0, [%r0, 4]
	brlt	%r0, %r1, clear_bss

	/* Zero the one and only argument of "board_init_f" */
	mov_s	%r0, 0
	j	board_init_f

memory_error:
	SAVE_ALL_SYS
	SAVE_EXCEPTION_SOURCE
	mov	%r1, %sp
	j	do_memory_error

instruction_error:
	SAVE_ALL_SYS
	SAVE_EXCEPTION_SOURCE
	mov	%r1, %sp
	j	do_instruction_error

interrupt_handler:
	/* Todo - save and restore CPU context when interrupts will be in use */
	bl	do_interrupt_handler
	rtie

EV_MachineCheck:
	SAVE_ALL_SYS
	SAVE_EXCEPTION_SOURCE
	mov	%r1, %sp
	j	do_machine_check_fault

EV_TLBMissI:
	SAVE_ALL_SYS
	mov	%r0, %sp
	j	do_itlb_miss

EV_TLBMissD:
	SAVE_ALL_SYS
	mov	%r0, %sp
	j	do_dtlb_miss

EV_TLBProtV:
	SAVE_ALL_SYS
	SAVE_EXCEPTION_SOURCE
	mov	%r1, %sp
	j	do_tlb_prot_violation

EV_PrivilegeV:
	SAVE_ALL_SYS
	mov	%r0, %sp
	j	do_privilege_violation

EV_Trap:
	SAVE_ALL_SYS
	mov	%r0, %sp
	j	do_trap

EV_Extension:
	SAVE_ALL_SYS
	mov	%r0, %sp
	j	do_extension

/*
 * void relocate_code (addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 * r0 = start_addr_sp
 * r1 = new__gd
 * r2 = relocaddr
 */
.align 4
.globl	relocate_code
relocate_code:
	/*
	 * r0-r12 might be clobbered by C functions
	 * so we use r13-r16 for storage here
	 */
	mov	%r13, %r0		/* save addr_sp */
	mov	%r14, %r1		/* save addr of gd */
	mov	%r15, %r2		/* save addr of destination */

	mov	%r16, %r2		/* %r9 - relocation offset */
	sub	%r16, %r16, __image_copy_start

/* Set up the stack */
stack_setup:
	mov	%sp, %r13
	mov	%fp, %sp

/* Check if monitor is loaded right in place for relocation */
	mov	%r0, __image_copy_start
	cmp	%r0, %r15		/* skip relocation if code loaded */
	bz	do_board_init_r		/* in target location already */

/* Copy data (__image_copy_start - __image_copy_end) to new location */
	mov	%r1, %r15
	mov	%r2, __image_copy_end
	sub	%r2, %r2, %r0		/* r3 <- amount of bytes to copy */
	asr	%r2, %r2, 2		/* r3 <- amount of words to copy */
	mov	%lp_count, %r2
	lp	copy_end
	ld.ab	%r2,[%r0,4]
	st.ab	%r2,[%r1,4]
copy_end:

/* Fix relocations related issues */
	bl	do_elf_reloc_fixups
#ifndef CONFIG_SYS_ICACHE_OFF
	bl	invalidate_icache_all
#endif
#ifndef CONFIG_SYS_DCACHE_OFF
	bl	flush_dcache_all
#endif

/* Update position of intterupt vector table */
	lr	%r0, [ARC_AUX_INTR_VEC_BASE]	/* Read current position */
	add	%r0, %r0, %r16			/* Update address */
	sr	%r0, [ARC_AUX_INTR_VEC_BASE]	/* Write new position */

do_board_init_r:
/* Prepare for exection of "board_init_r" in relocated monitor */
	mov	%r2, board_init_r	/* old address of "board_init_r()" */
	add	%r2, %r2, %r16		/* new address of "board_init_r()" */
	mov	%r0, %r14		/* 1-st parameter: gd_t */
	mov	%r1, %r15		/* 2-nd parameter: dest_addr */
	j	[%r2]
