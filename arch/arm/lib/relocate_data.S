/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  relocate - data relocation function for ARM U-Boot
 *
 *  Copyright (c) 2013  Albert ARIBAUD <albert.u.boot@aribaud.net>
 */

#include <asm-offsets.h>
#include <asm/assembler.h>
#include <config.h>
#include <elf.h>
#include <linux/linkage.h>
#ifdef CONFIG_CPU_V7M
#include <asm/armv7m.h>
#endif

/*
 * Default/weak exception vectors relocation routine
 *
 * This routine covers the standard ARM cases: normal (0x00000000),
 * high (0xffff0000) and VBAR. SoCs which do not comply with any of
 * the standard cases must provide their own, strong, version.
 */

	.section	.text.relocate_vectors,"ax",%progbits
	.weak		relocate_vectors

ENTRY(relocate_vectors)
	bx	lr

ENDPROC(relocate_vectors)

/*
 * void relocate_data()
 *
 * This function relocates the initialized data.
 *
 */

ENTRY(relocate_data)
	ldr	r0, =__data_in_flash
	ldr	r1, =_sdata
	ldr	r2, =_edata
copy_loop:
	ldmia	r0!, {r10-r11}      /* copy from source address [r1] */
	stmia	r1!, {r10-r11}      /* copy to   target address [r0] */
	cmp	r1, r2      	        /* until source end address [r2] */
	blo	copy_loop

	bx	lr

ENDPROC(relocate_data)
