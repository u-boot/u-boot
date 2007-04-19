/*
 *  Copyright (C) 2002 Wolfgang Denk <wd@denx.de>
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

#ifdef CONFIG_POST
#if defined(CONFIG_MPC823) || \
    defined(CONFIG_MPC850) || \
    defined(CONFIG_MPC855) || \
    defined(CONFIG_MPC860) || \
    defined(CONFIG_MPC862)

#include <post.h>
#include <ppc_asm.tmpl>
#include <ppc_defs.h>
#include <asm/cache.h>

#if CONFIG_POST & CFG_POST_CACHE

	.text

cache_post_dinvalidate:
	lis	r10, IDC_INVALL@h
	mtspr	DC_CST, r10
	blr

cache_post_iinvalidate:
	lis	r10, IDC_INVALL@h
	mtspr	IC_CST, r10
	isync
	blr

cache_post_ddisable:
	lis	r10, IDC_DISABLE@h
	mtspr	DC_CST, r10
	blr

cache_post_dwb:
	lis	r10, IDC_ENABLE@h
	mtspr	DC_CST, r10
	lis	r10, DC_CFWT@h
	mtspr	DC_CST, r10
	blr

cache_post_dwt:
	lis	r10, IDC_ENABLE@h
	mtspr	DC_CST, r10
	lis	r10, DC_SFWT@h
	mtspr	DC_CST, r10
	blr

cache_post_idisable:
	lis	r10, IDC_DISABLE@h
	mtspr	IC_CST, r10
	isync
	blr

cache_post_ienable:
	lis	r10, IDC_ENABLE@h
	mtspr	IC_CST, r10
	isync
	blr

cache_post_iunlock:
	lis	r10, IDC_UNALL@h
	mtspr	IC_CST, r10
	isync
	blr

cache_post_ilock:
	mtspr	IC_ADR, r3
	lis	r10, IDC_LDLCK@h
	mtspr	IC_CST, r10
	isync
	blr

/*
 * turn on the data cache
 * switch the data cache to write-back or write-through mode
 * invalidate the data cache
 * write the negative pattern to a cached area
 * read the area
 *
 * The negative pattern must be read at the last step
 */
	.global cache_post_test1
cache_post_test1:
	mflr	r0
	stw	r0, 4(r1)

	stwu	r3, -4(r1)
	stwu	r4, -4(r1)

	bl	cache_post_dwb
	bl	cache_post_dinvalidate

	/* Write the negative pattern to the test area */
	lwz	r0, 0(r1)
	mtctr	r0
	li	r0, 0xff
	lwz	r3, 4(r1)
	subi	r3, r3, 1
1:
	stbu	r0, 1(r3)
	bdnz	1b

	/* Read the test area */
	lwz	r0, 0(r1)
	mtctr	r0
	lwz	r4, 4(r1)
	subi	r4, r4, 1
	li	r3, 0
1:
	lbzu	r0, 1(r4)
	cmpli	cr0, r0, 0xff
	beq	2f
	li	r3, -1
	b	3f
2:
	bdnz	1b
3:

	bl	cache_post_ddisable
	bl	cache_post_dinvalidate

	addi	r1, r1, 8

	lwz	r0, 4(r1)
	mtlr	r0
	blr

/*
 * turn on the data cache
 * switch the data cache to write-back or write-through mode
 * invalidate the data cache
 * write the zero pattern to a cached area
 * turn off the data cache
 * write the negative pattern to the area
 * turn on the data cache
 * read the area
 *
 * The negative pattern must be read at the last step
 */
	.global cache_post_test2
cache_post_test2:
	mflr	r0
	stw	r0, 4(r1)

	stwu	r3, -4(r1)
	stwu	r4, -4(r1)

	bl	cache_post_dwb
	bl	cache_post_dinvalidate

	/* Write the zero pattern to the test area */
	lwz	r0, 0(r1)
	mtctr	r0
	li	r0, 0
	lwz	r3, 4(r1)
	subi	r3, r3, 1
1:
	stbu	r0, 1(r3)
	bdnz	1b

	bl	cache_post_ddisable

	/* Write the negative pattern to the test area */
	lwz	r0, 0(r1)
	mtctr	r0
	li	r0, 0xff
	lwz	r3, 4(r1)
	subi	r3, r3, 1
1:
	stbu	r0, 1(r3)
	bdnz	1b

	bl	cache_post_dwb

	/* Read the test area */
	lwz	r0, 0(r1)
	mtctr	r0
	lwz	r4, 4(r1)
	subi	r4, r4, 1
	li	r3, 0
1:
	lbzu	r0, 1(r4)
	cmpli	cr0, r0, 0xff
	beq	2f
	li	r3, -1
	b	3f
2:
	bdnz	1b
3:

	bl	cache_post_ddisable
	bl	cache_post_dinvalidate

	addi	r1, r1, 8

	lwz	r0, 4(r1)
	mtlr	r0
	blr

/*
 * turn on the data cache
 * switch the data cache to write-through mode
 * invalidate the data cache
 * write the zero pattern to a cached area
 * flush the data cache
 * write the negative pattern to the area
 * turn off the data cache
 * read the area
 *
 * The negative pattern must be read at the last step
 */
	.global cache_post_test3
cache_post_test3:
	mflr	r0
	stw	r0, 4(r1)

	stwu	r3, -4(r1)
	stwu	r4, -4(r1)

	bl	cache_post_ddisable
	bl	cache_post_dinvalidate

	/* Write the zero pattern to the test area */
	lwz	r0, 0(r1)
	mtctr	r0
	li	r0, 0
	lwz	r3, 4(r1)
	subi	r3, r3, 1
1:
	stbu	r0, 1(r3)
	bdnz	1b

	bl	cache_post_dwt
	bl	cache_post_dinvalidate

	/* Write the negative pattern to the test area */
	lwz	r0, 0(r1)
	mtctr	r0
	li	r0, 0xff
	lwz	r3, 4(r1)
	subi	r3, r3, 1
1:
	stbu	r0, 1(r3)
	bdnz	1b

	bl	cache_post_ddisable
	bl	cache_post_dinvalidate

	/* Read the test area */
	lwz	r0, 0(r1)
	mtctr	r0
	lwz	r4, 4(r1)
	subi	r4, r4, 1
	li	r3, 0
1:
	lbzu	r0, 1(r4)
	cmpli	cr0, r0, 0xff
	beq	2f
	li	r3, -1
	b	3f
2:
	bdnz	1b
3:

	addi	r1, r1, 8

	lwz	r0, 4(r1)
	mtlr	r0
	blr

/*
 * turn on the data cache
 * switch the data cache to write-back mode
 * invalidate the data cache
 * write the negative pattern to a cached area
 * flush the data cache
 * write the zero pattern to the area
 * invalidate the data cache
 * read the area
 *
 * The negative pattern must be read at the last step
 */
	.global cache_post_test4
cache_post_test4:
	mflr	r0
	stw	r0, 4(r1)

	stwu	r3, -4(r1)
	stwu	r4, -4(r1)

	bl	cache_post_ddisable
	bl	cache_post_dinvalidate

	/* Write the negative pattern to the test area */
	lwz	r0, 0(r1)
	mtctr	r0
	li	r0, 0xff
	lwz	r3, 4(r1)
	subi	r3, r3, 1
1:
	stbu	r0, 1(r3)
	bdnz	1b

	bl	cache_post_dwb
	bl	cache_post_dinvalidate

	/* Write the zero pattern to the test area */
	lwz	r0, 0(r1)
	mtctr	r0
	li	r0, 0
	lwz	r3, 4(r1)
	subi	r3, r3, 1
1:
	stbu	r0, 1(r3)
	bdnz	1b

	bl	cache_post_ddisable
	bl	cache_post_dinvalidate

	/* Read the test area */
	lwz	r0, 0(r1)
	mtctr	r0
	lwz	r4, 4(r1)
	subi	r4, r4, 1
	li	r3, 0
1:
	lbzu	r0, 1(r4)
	cmpli	cr0, r0, 0xff
	beq	2f
	li	r3, -1
	b	3f
2:
	bdnz	1b
3:

	addi	r1, r1, 8

	lwz	r0, 4(r1)
	mtlr	r0
	blr

cache_post_test5_1:
	li	r3, 0
cache_post_test5_2:
	li	r3, -1

/*
 * turn on the instruction cache
 * unlock the entire instruction cache
 * invalidate the instruction cache
 * lock a branch instruction in the instruction cache
 * replace the branch instruction with "nop"
 * jump to the branch instruction
 * check that the branch instruction was executed
*/
	.global cache_post_test5
cache_post_test5:
	mflr	r0
	stw	r0, 4(r1)

	bl	cache_post_ienable
	bl	cache_post_iunlock
	bl	cache_post_iinvalidate

	/* Compute r9 = cache_post_test5_reloc */
	bl	cache_post_test5_reloc
cache_post_test5_reloc:
	mflr	r9

	/* Copy the test instruction to cache_post_test5_data */
	lis	r3, (cache_post_test5_1 - cache_post_test5_reloc)@h
	ori	r3, r3, (cache_post_test5_1 - cache_post_test5_reloc)@l
	add	r3, r3, r9
	lis	r4, (cache_post_test5_data - cache_post_test5_reloc)@h
	ori	r4, r4, (cache_post_test5_data - cache_post_test5_reloc)@l
	add	r4, r4, r9
	lwz	r0, 0(r3)
	stw	r0, 0(r4)

	bl	cache_post_iinvalidate

	/* Lock the branch instruction */
	lis	r3, (cache_post_test5_data - cache_post_test5_reloc)@h
	ori	r3, r3, (cache_post_test5_data - cache_post_test5_reloc)@l
	add	r3, r3, r9
	bl	cache_post_ilock

	/* Replace the test instruction */
	lis	r3, (cache_post_test5_2 - cache_post_test5_reloc)@h
	ori	r3, r3, (cache_post_test5_2 - cache_post_test5_reloc)@l
	add	r3, r3, r9
	lis	r4, (cache_post_test5_data - cache_post_test5_reloc)@h
	ori	r4, r4, (cache_post_test5_data - cache_post_test5_reloc)@l
	add	r4, r4, r9
	lwz	r0, 0(r3)
	stw	r0, 0(r4)

	bl	cache_post_iinvalidate

	/* Execute to the test instruction */
cache_post_test5_data:
	nop

	bl	cache_post_iunlock

	lwz	r0, 4(r1)
	mtlr	r0
	blr

cache_post_test6_1:
	li	r3, -1
cache_post_test6_2:
	li	r3, 0

/*
 * turn on the instruction cache
 * unlock the entire instruction cache
 * invalidate the instruction cache
 * lock a branch instruction in the instruction cache
 * replace the branch instruction with "nop"
 * jump to the branch instruction
 * check that the branch instruction was executed
 */
	.global cache_post_test6
cache_post_test6:
	mflr	r0
	stw	r0, 4(r1)

	bl	cache_post_ienable
	bl	cache_post_iunlock
	bl	cache_post_iinvalidate

	/* Compute r9 = cache_post_test6_reloc */
	bl	cache_post_test6_reloc
cache_post_test6_reloc:
	mflr	r9

	/* Copy the test instruction to cache_post_test6_data */
	lis	r3, (cache_post_test6_1 - cache_post_test6_reloc)@h
	ori	r3, r3, (cache_post_test6_1 - cache_post_test6_reloc)@l
	add	r3, r3, r9
	lis	r4, (cache_post_test6_data - cache_post_test6_reloc)@h
	ori	r4, r4, (cache_post_test6_data - cache_post_test6_reloc)@l
	add	r4, r4, r9
	lwz	r0, 0(r3)
	stw	r0, 0(r4)

	bl	cache_post_iinvalidate

	/* Replace the test instruction */
	lis	r3, (cache_post_test6_2 - cache_post_test6_reloc)@h
	ori	r3, r3, (cache_post_test6_2 - cache_post_test6_reloc)@l
	add	r3, r3, r9
	lis	r4, (cache_post_test6_data - cache_post_test6_reloc)@h
	ori	r4, r4, (cache_post_test6_data - cache_post_test6_reloc)@l
	add	r4, r4, r9
	lwz	r0, 0(r3)
	stw	r0, 0(r4)

	bl	cache_post_iinvalidate

	/* Execute to the test instruction */
cache_post_test6_data:
	nop

	lwz	r0, 4(r1)
	mtlr	r0
	blr

#endif /* CONFIG_MPC823 || MPC850 || MPC855 || MPC860 */
#endif /* CONFIG_POST & CFG_POST_CACHE */
#endif /* CONFIG_POST */
