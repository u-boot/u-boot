/*
 * Copyright (C) 2005-2008 Atmel Corporation
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
#include <asm/ptrace.h>
#include <asm/sysreg.h>

#define SYSREG_MMUCR_I_OFFSET	2
#define SYSREG_MMUCR_S_OFFSET	4

#define SR_INIT (SYSREG_BIT(GM) | SYSREG_BIT(EM) | SYSREG_BIT(M0))
/* due to errata (unreliable branch folding) clear FE bit explicitly */
#define CPUCR_INIT ((SYSREG_BIT(BI) | SYSREG_BIT(BE)	\
		    | SYSREG_BIT(RE)   |  SYSREG_BIT(IBE)		\
		    | SYSREG_BIT(IEE)) & ~SYSREG_BIT(FE))

	/*
	 * To save some space, we use the same entry point for
	 * exceptions and reset. This avoids lots of alignment padding
	 * since the reset vector is always suitably aligned.
	 */
	.section .exception.text, "ax", @progbits
	.global	_start
	.global	_evba
	.type	_start, @function
	.type	_evba, @function
_start:
	.size	_start, 0
_evba:
	.org	0x00
	rjmp	unknown_exception	/* Unrecoverable exception */
	.org	0x04
	rjmp	unknown_exception	/* TLB multiple hit */
	.org	0x08
	rjmp	unknown_exception	/* Bus error data fetch */
	.org	0x0c
	rjmp	unknown_exception	/* Bus error instruction fetch */
	.org	0x10
	rjmp	unknown_exception	/* NMI */
	.org	0x14
	rjmp	unknown_exception	/* Instruction address */
	.org	0x18
	rjmp	unknown_exception	/* ITLB protection */
	.org	0x1c
	rjmp	unknown_exception	/* Breakpoint */
	.org	0x20
	rjmp	unknown_exception	/* Illegal opcode */
	.org	0x24
	rjmp	unknown_exception	/* Unimplemented instruction */
	.org	0x28
	rjmp	unknown_exception	/* Privilege violation */
	.org	0x2c
	rjmp	unknown_exception	/* Floating-point */
	.org	0x30
	rjmp	unknown_exception	/* Coprocessor absent */
	.org	0x34
	rjmp	unknown_exception	/* Data Address (read) */
	.org	0x38
	rjmp	unknown_exception	/* Data Address (write) */
	.org	0x3c
	rjmp	unknown_exception	/* DTLB Protection (read) */
	.org	0x40
	rjmp	unknown_exception	/* DTLB Protection (write) */
	.org	0x44
	rjmp	unknown_exception	/* DTLB Modified */

	.org	0x50			/* ITLB Miss */
	pushm   r8-r12,lr
	rjmp	1f
	.org	0x60			/* DTLB Miss (read) */
	pushm   r8-r12,lr
	rjmp	1f
	.org	0x70			/* DTLB Miss (write) */
	pushm   r8-r12,lr
1:	mov	r12, sp
	rcall	mmu_handle_tlb_miss
	popm	r8-r12,lr
	brne	unknown_exception
	rete

	.size	_evba, . - _evba

	.align	2
	.type	unknown_exception, @function
unknown_exception:
	/* Figure out whether we're handling an exception (Exception
	 * mode) or just booting (Supervisor mode). */
	csrfcz	SYSREG_M1_OFFSET
	brcc	at32ap_cpu_bootstrap

	/* This is an exception. Complain. */
	pushm	r0-r12
	sub	r8, sp, REG_R12 - REG_R0 - 4
	mov	r9, lr
	mfsr	r10, SYSREG_RAR_EX
	mfsr	r11, SYSREG_RSR_EX
	pushm	r8-r11
	mfsr	r12, SYSREG_ECR
	mov	r11, sp
	rcall	do_unknown_exception
1:	rjmp	1b

	/* The COUNT/COMPARE timer interrupt handler */
	.global	timer_interrupt_handler
	.type	timer_interrupt_handler,@function
	.align	2
timer_interrupt_handler:
	/*
	 * Increment timer_overflow and re-write COMPARE with 0xffffffff.
	 *
	 * We're running at interrupt level 3, so we don't need to save
	 * r8-r12 or lr to the stack.
	 */
	lda.w	r8, timer_overflow
	ld.w	r9, r8[0]
	mov	r10, -1
	mtsr	SYSREG_COMPARE, r10
	sub	r9, -1
	st.w	r8[0], r9
	rete

	/*
	 * CPU bootstrap after reset is handled here. SoC code may
	 * override this in case they need to initialize oscillators,
	 * etc.
	 */
	.section .text.at32ap_cpu_bootstrap, "ax", @progbits
	.global	at32ap_cpu_bootstrap
	.weak	at32ap_cpu_bootstrap
	.type	at32ap_cpu_bootstrap, @function
	.align	2
at32ap_cpu_bootstrap:
	/* Reset the Status Register */
	mov	r0, lo(SR_INIT)
	orh	r0, hi(SR_INIT)
	mtsr	SYSREG_SR, r0

	/* Reset CPUCR and invalidate the BTB */
	mov	r2, CPUCR_INIT
	mtsr	SYSREG_CPUCR, r2

	/* Flush the caches */
	mov	r1, 0
	cache	r1[4], 8
	cache	r1[0], 0
	sync	0

	/* Reset the MMU to default settings */
	mov	r0, SYSREG_BIT(MMUCR_S) | SYSREG_BIT(MMUCR_I)
	mtsr	SYSREG_MMUCR, r0

	/* Internal RAM should not need any initialization.  We might
	   have to initialize external RAM here if the part doesn't
	   have internal RAM (or we may use the data cache) */

	/* Jump to cacheable segment */
	lddpc	pc, 1f

	.align	2
1:	.long	at32ap_low_level_init
	.size	_start, . - _start

	/* Common CPU bootstrap code after oscillator/cache/etc. init */
	.section .text.avr32ap_low_level_init, "ax", @progbits
	.global	at32ap_low_level_init
	.type	at32ap_low_level_init, @function
	.align	2
at32ap_low_level_init:
	lddpc	sp, sp_init

	/* Initialize the GOT pointer */
	lddpc	r6, got_init
3:	rsub	r6, pc

	/* Let's go */
	rjmp	board_init_f

	.align	2
	.type	sp_init,@object
sp_init:
	.long	CONFIG_SYS_INIT_SP_ADDR
got_init:
	.long	3b - _GLOBAL_OFFSET_TABLE_

	/*
	 * void	relocate_code(new_sp, new_gd, monitor_addr)
	 *
	 * Relocate the u-boot image into RAM and continue from there.
	 * Does not return.
	 */
	.section .text.relocate_code,"ax",@progbits
	.global	relocate_code
	.type	relocate_code,@function
relocate_code:
	mov	sp, r12		/* use new stack */
	mov	r12, r11	/* save new_gd */
	mov	r11, r10	/* save destination address */

	/* copy .text section and flush the cache along the way */
	lda.w	r8, _text
	lda.w	r9, _etext
	sub	lr, r10, r8	/* relocation offset */

1:	ldm	r8++, r0-r3
	stm	r10, r0-r3
	sub	r10, -16
	ldm	r8++, r0-r3
	stm	r10, r0-r3
	sub	r10, -16
	cp.w	r8, r9
	cache	r10[-4], 0x0d	/* dcache clean/invalidate */
	cache	r10[-4], 0x01	/* icache invalidate */
	brlt	1b

	/* flush write buffer */
	sync	0

	/* copy data sections */
	lda.w	r9, _edata
1:	ld.d	r0, r8++
	st.d	r10++, r0
	cp.w	r8, r9
	brlt	1b

	/* zero out .bss */
	mov	r0, 0
	mov	r1, 0
	lda.w	r9, _end
	sub	r9, r8
1:	st.d	r10++, r0
	sub	r9, 8
	brgt	1b

	/* jump to RAM */
	sub	r0, pc, . - in_ram
	add	pc, r0, lr

	.align	2
in_ram:
	/* find the new GOT and relocate it */
	lddpc	r6, got_init_reloc
3:	rsub	r6, pc
	mov	r8, r6
	lda.w	r9, _egot
	lda.w	r10, _got
	sub	r9, r10
1:	ld.w	r0, r8[0]
	add	r0, lr
	st.w	r8++, r0
	sub	r9, 4
	brgt	1b

	/* Move the exception handlers */
	mfsr	r2, SYSREG_EVBA
	add	r2, lr
	mtsr	SYSREG_EVBA, r2

	/* Do the rest of the initialization sequence */
	call	board_init_r

	.align	2
got_init_reloc:
	.long	3b - _GLOBAL_OFFSET_TABLE_

	.size	relocate_code, . - relocate_code
