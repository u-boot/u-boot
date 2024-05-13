/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  relocate - S5P4418 specific relocation for ARM U-Boot
 *
 *  Copyright (c) 2013  Albert ARIBAUD <albert.u.boot@aribaud.net>
 *  Copyright (C) 2023  Stefan Bosch <stefan_b@posteo.net>
 */

#include <asm-offsets.h>
#include <asm/assembler.h>
#include <linux/linkage.h>

ENTRY(relocate_vectors)

	/*
	 * The s5p4418 SoC has the security extensions, so use VBAR to relocate
	 * the exception vectors.
	 */
	ldr	r0, [r9, #GD_RELOCADDR]	/* r0 = gd->relocaddr */
	add	r0, #0x400			/* vectors are after NSIH + 0x200 */
	mcr	p15, 0, r0, c12, c0, 0	/* Set VBAR */
	ret	lr

ENDPROC(relocate_vectors)
