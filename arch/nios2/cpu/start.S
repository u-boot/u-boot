/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <version.h>

/*************************************************************************
 * RESTART
 ************************************************************************/

	.text
	.global _start

_start:
	wrctl	status, r0		/* Disable interrupts */
	/* ICACHE INIT -- only the icache line at the reset address
	 * is invalidated at reset. So the init must stay within
	 * the cache line size (8 words). If GERMS is used, we'll
	 * just be invalidating the cache a second time. If cache
	 * is not implemented initi behaves as nop.
	 */
	ori	r4, r0, %lo(CONFIG_SYS_ICACHELINE_SIZE)
	movhi	r5, %hi(CONFIG_SYS_ICACHE_SIZE)
	ori	r5, r5, %lo(CONFIG_SYS_ICACHE_SIZE)
0:	initi	r5
	sub	r5, r5, r4
	bgt	r5, r0, 0b
	br	_except_end	/* Skip the tramp */

	/* EXCEPTION TRAMPOLINE -- the following gets copied
	 * to the exception address (below), but is otherwise at the
	 * default exception vector offset (0x0020).
	 */
_except_start:
	movhi	et, %hi(_exception)
	ori	et, et, %lo(_exception)
	jmp	et
_except_end:

	/* INTERRUPTS -- for now, all interrupts masked and globally
	 * disabled.
	 */
	wrctl	ienable, r0		/* All disabled	*/

	/* DCACHE INIT -- if dcache not implemented, initd behaves as
	 * nop.
	 */
	movhi	r4, %hi(CONFIG_SYS_DCACHELINE_SIZE)
	ori	r4, r4, %lo(CONFIG_SYS_DCACHELINE_SIZE)
	movhi	r5, %hi(CONFIG_SYS_DCACHE_SIZE)
	ori	r5, r5, %lo(CONFIG_SYS_DCACHE_SIZE)
	mov	r6, r0
1:	initd	0(r6)
	add	r6, r6, r4
	bltu	r6, r5, 1b

	/* RELOCATE CODE, DATA & COMMAND TABLE -- the following code
	 * assumes code, data and the command table are all
	 * contiguous. This lets us relocate everything as a single
	 * block. Make sure the linker script matches this ;-)
	 */
	nextpc	r4
_cur:	movhi	r5, %hi(_cur - _start)
	ori	r5, r5, %lo(_cur - _start)
	sub	r4, r4, r5		/* r4 <- cur _start */
	mov	r8, r4
	movhi	r5, %hi(_start)
	ori	r5, r5, %lo(_start)	/* r5 <- linked _start */
	beq	r4, r5, 3f

	movhi	r6, %hi(CONFIG_SYS_MONITOR_LEN)
	ori	r6, r6, %lo(CONFIG_SYS_MONITOR_LEN)
	add	r6, r6, r5
2:	ldwio	r7, 0(r4)
	addi	r4, r4, 4
	stwio	r7, 0(r5)
	addi	r5, r5, 4
	bne	r5, r6, 2b
3:

	/* JUMP TO RELOC ADDR */
	movhi	r4, %hi(_reloc)
	ori	r4, r4, %lo(_reloc)
	jmp	r4
_reloc:

	/* COPY EXCEPTION TRAMPOLINE -- copy the tramp to the
	 * exception address. Define CONFIG_ROM_STUBS to prevent
	 * the copy (e.g. exception in flash or in other
	 * softare/firmware component).
	 */
#if !defined(CONFIG_ROM_STUBS)
	movhi	r4, %hi(_except_start)
	ori	r4, r4, %lo(_except_start)
	movhi	r5, %hi(_except_end)
	ori	r5, r5, %lo(_except_end)
	movhi	r6, %hi(CONFIG_SYS_EXCEPTION_ADDR)
	ori	r6, r6, %lo(CONFIG_SYS_EXCEPTION_ADDR)
	beq	r4, r6, 7f	/* Skip if at proper addr */

6:	ldwio	r7, 0(r4)
	stwio	r7, 0(r6)
	addi	r4, r4, 4
	addi	r6, r6, 4
	bne	r4, r5, 6b
7:
#endif

	/* STACK INIT -- zero top two words for call back chain.
	 */
	movhi	sp, %hi(CONFIG_SYS_INIT_SP)
	ori	sp, sp, %lo(CONFIG_SYS_INIT_SP)
	addi	sp, sp, -8
	stw	r0, 0(sp)
	stw	r0, 4(sp)
	mov	fp, sp

	/*
	 * Call board_init_f -- never returns
	 */
	mov	r4, r0
	movhi	r2, %hi(board_init_f@h)
	ori	r2, r2, %lo(board_init_f@h)
	callr	r2

	/* NEVER RETURNS -- but branch to the _start just
	 * in case ;-)
	 */
	br	_start



/*
 * relocate_code -- Nios2 handles the relocation above. But
 * the generic board code monkeys with the heap, stack, etc.
 * (it makes some assumptions that may not be appropriate
 * for Nios). Nevertheless, we capitulate here.
 *
 * We'll call the board_init_r from here since this isn't
 * supposed to return.
 *
 * void relocate_code (ulong sp, gd_t *global_data,
 *			ulong reloc_addr)
 *			__attribute__ ((noreturn));
 */
	.text
	.global relocate_code

relocate_code:
	mov	sp, r4		/* Set the new sp */
	mov	r4, r5

	/*
	 * ZERO BSS/SBSS -- bss and sbss are assumed to be adjacent
	 * and between __bss_start and __bss_end.
	 */
	movhi	r5, %hi(__bss_start)
	ori	r5, r5, %lo(__bss_start)
	movhi	r6, %hi(__bss_end)
	ori	r6, r6, %lo(__bss_end)
	beq	r5, r6, 5f

4:	stwio	r0, 0(r5)
	addi	r5, r5, 4
	bne	r5, r6, 4b
5:

	movhi	r8, %hi(board_init_r@h)
	ori	r8, r8, %lo(board_init_r@h)
	callr	r8
	ret
