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

	/*
	 * Relocate the data section and initialize .bss.  Everything
	 * is guaranteed to be at least doubleword aligned by the
	 * linker script.
	 */
	lddpc	r12, .Ldata_vma
	lddpc	r11, .Ldata_lma
	lddpc	r10, .Ldata_end
	sub	r10, r12
4:	ld.d	r8, r11++
	sub	r10, 8
	st.d	r12++, r8
	brne	4b

	mov	r8, 0
	mov	r9, 0
	lddpc	r10, .Lbss_end
	sub	r10, r12
4:	sub	r10, 8
	st.d	r12++, r8
	brne	4b

	/* Initialize the GOT pointer */
	lddpc	r6, got_init
3:	rsub	r6, pc
	ld.w	pc, r6[start_u_boot@got]

	.align	2
	.type	sp_init,@object
sp_init:
	.long	CFG_INIT_SP_ADDR
got_init:
	.long	3b - _GLOBAL_OFFSET_TABLE_
.Ldata_lma:
	.long	__data_lma
.Ldata_vma:
	.long	_data
.Ldata_end:
	.long	_edata
.Lbss_end:
	.long	_end
