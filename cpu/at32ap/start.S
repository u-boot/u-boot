/*
 * Copyright (C) 2005-2006 Atmel Corporation
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
#include <asm/sysreg.h>

#ifndef PART_SPECIFIC_BOOTSTRAP
# define PART_SPECIFIC_BOOTSTRAP
#endif

#define SYSREG_MMUCR_I_OFFSET	2
#define SYSREG_MMUCR_S_OFFSET	4

#define SR_INIT (SYSREG_BIT(GM) | SYSREG_BIT(EM) | SYSREG_BIT(M0))
#define CPUCR_INIT (SYSREG_BIT(BI) | SYSREG_BIT(BE)		\
		    | SYSREG_BIT(FE) | SYSREG_BIT(RE)		\
		    | SYSREG_BIT(IBE) | SYSREG_BIT(IEE))

	.text
	.global	_start
_start:
	PART_SPECIFIC_BOOTSTRAP

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
1:	.long	2f

2:	lddpc	sp, sp_init

	/* Initialize the GOT pointer */
	lddpc	r6, got_init
3:	rsub	r6, pc

	/* Let's go */
	rjmp	board_init_f

	.align	2
	.type	sp_init,@object
sp_init:
	.long	CFG_INIT_SP_ADDR
got_init:
	.long	3b - _GLOBAL_OFFSET_TABLE_

	/*
	 * void	relocate_code(new_sp, new_gd, monitor_addr)
	 *
	 * Relocate the u-boot image into RAM and continue from there.
	 * Does not return.
	 */
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
