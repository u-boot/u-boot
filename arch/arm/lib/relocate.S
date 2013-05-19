/*
 *  relocate - common relocation function for ARM U-Boot
 *
 *  Copyright (c) 2013  Albert ARIBAUD <albert.u.boot@aribaud.net>
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

#include <linux/linkage.h>

/*
 * void relocate_code(addr_moni)
 *
 * This function relocates the monitor code.
 *
 * NOTE:
 * To prevent the code below from containing references with an R_ARM_ABS32
 * relocation record type, we never refer to linker-defined symbols directly.
 * Instead, we declare literals which contain their relative location with
 * respect to relocate_code, and at run time, add relocate_code back to them.
 */

ENTRY(relocate_code)
	mov	r6, r0	/* save addr of destination */

	ldr	r0, =_start		/* r0 <- SRC &_start */
	subs	r9, r6, r0		/* r9 <- relocation offset */
	beq	relocate_done		/* skip relocation */
	mov	r1, r6			/* r1 <- scratch for copy loop */
	adr	r7, relocate_code	/* r7 <- SRC &relocate_code */
	ldr	r3, _image_copy_end_ofs	/* r3 <- __image_copy_end local ofs */
	add	r2, r7, r3		/* r2 <- SRC &__image_copy_end */

copy_loop:
	ldmia	r0!, {r10-r11}		/* copy from source address [r0]    */
	stmia	r1!, {r10-r11}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end address [r2]    */
	blo	copy_loop

	/*
	 * fix .rel.dyn relocations
	 */
	ldr	r10, _dynsym_start_ofs	/* r10 <- __dynsym_start local ofs */
	add	r10, r10, r7		/* r10 <- SRC &__dynsym_start */
	ldr	r2, _rel_dyn_start_ofs	/* r2 <- __rel_dyn_start local ofs */
	add	r2, r2, r7		/* r2 <- SRC &__rel_dyn_start */
	ldr	r3, _rel_dyn_end_ofs	/* r3 <- __rel_dyn_end local ofs */
	add	r3, r3, r7		/* r3 <- SRC &__rel_dyn_end */
fixloop:
	ldr	r0, [r2]		/* r0 <- SRC location to fix up */
	add	r0, r0, r9		/* r0 <- DST location to fix up */
	ldr	r1, [r2, #4]
	and	r7, r1, #0xff
	cmp	r7, #23			/* relative fixup? */
	beq	fixrel
	cmp	r7, #2			/* absolute fixup? */
	beq	fixabs
	/* ignore unknown type of fixup */
	b	fixnext
fixabs:
	/* absolute fix: set location to (offset) symbol value */
	mov	r1, r1, LSR #4		/* r1 <- symbol index in .dynsym */
	add	r1, r10, r1		/* r1 <- address of symbol in table */
	ldr	r1, [r1, #4]		/* r1 <- symbol value */
	add	r1, r1, r9		/* r1 <- relocated sym addr */
	b	fixnext
fixrel:
	/* relative fix: increase location by offset */
	ldr	r1, [r0]
	add	r1, r1, r9
fixnext:
	str	r1, [r0]
	add	r2, r2, #8		/* each rel.dyn entry is 8 bytes */
	cmp	r2, r3
	blo	fixloop

relocate_done:

	/* ARMv4- don't know bx lr but the assembler fails to see that */

#ifdef __ARM_ARCH_4__
        mov        pc, lr
#else
        bx        lr
#endif

_image_copy_end_ofs:
	.word __image_copy_end - relocate_code
_rel_dyn_start_ofs:
	.word __rel_dyn_start - relocate_code
_rel_dyn_end_ofs:
	.word __rel_dyn_end - relocate_code
_dynsym_start_ofs:
	.word __dynsym_start - relocate_code

ENDPROC(relocate_code)
