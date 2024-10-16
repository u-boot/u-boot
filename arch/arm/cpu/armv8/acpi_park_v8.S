/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 9elements GmbH
 *   Author: Patrick Rudolph <patrick.rudolph@9elements.com>
 *
 * This file provides ARMv8 specific code for the generic part of the
 * ACPI parking protocol implementation. It contains the spinning code
 * that will be installed into the parking protocol and it points the
 * secondary CPUs to their own parking protocol page once it has been
 * set up by the generic part.
 */

#include <asm/acpi_table.h>
#include <linux/linkage.h>

/* Filled by C code */
.global acpi_pp_tables
acpi_pp_tables:
	.quad 0

.global acpi_pp_etables
acpi_pp_etables:
	.quad 0

/* Read by C code */
.global acpi_pp_code_size
acpi_pp_code_size:
	.word __secondary_pp_code_end - __secondary_pp_code_start

.global acpi_pp_secondary_jump
ENTRY(acpi_pp_secondary_jump)
0:
	/*
	 * Cannot use atomic operations since the MMU and D-cache
	 * might be off. Use the MPIDR instead to find the spintable.
	 */

	/* Check if parking protocol table is ready */
	ldr	x1, =acpi_pp_tables
	ldr	x0, [x1]
	cbnz	x0, 0f
	wfe
	b	0b

0:	/* Get end of page tables in x3 */
	ldr	x1, =acpi_pp_etables
	ldr	x3, [x1]

	/* Get own CPU ID in w2 */
	mrs	x2, mpidr_el1
	lsr	x9, x2, #32
	bfi	x2, x9, #24, #8	/* w2 is aff3:aff2:aff1:aff0 */

0:	/* Loop over all parking protocol pages */
	cmp	x0, x3
	b.ge	hlt

	/* Fetch CPU_ID from current page */
	ldr	x1, [x0, #ACPI_PP_CPU_ID_OFFSET]
	lsr	x9, x1, #32
	bfi	x1, x9, #24, #8	/* w1 is aff3:aff2:aff1:aff0 */

	/* Compare CPU_IDs */
	cmp	w1, w2
	b.eq	0f

	add	x0, x0, #ACPI_PP_PAGE_SIZE
	b	0b

hlt:	wfi
	b	hlt	/* Should never happen. */

0:	/* x0 points to the 4K-aligned, parking protocol page */
	add	x2, x0, #ACPI_PP_CPU_CODE_OFFSET

	/* Jump to spin code in own parking protocol page */
	br	x2
ENDPROC(acpi_pp_secondary_jump)

.align 8
__secondary_pp_code_start:
.global acpi_pp_code_start
ENTRY(acpi_pp_code_start)
	/* x0 points to the 4K-aligned, parking protocol page */

	/* Prepare defines for spinning code */
	mov	w3, #ACPI_PP_CPU_ID_INVALID
	mov	x2, #ACPI_PP_JMP_ADR_INVALID

	/* Mark parking protocol page as ready */
	str	w3, [x0, #ACPI_PP_CPU_ID_OFFSET]
	dsb	sy

0:	wfe
	ldr	w1, [x0, #ACPI_PP_CPU_ID_OFFSET]

	/* Check CPU ID is valid */
	cmp	w1, w3
	b.eq	0b

	/* Check jump address valid */
	ldr	x1, [x0, #ACPI_PP_CPU_JMP_OFFSET]
	cmp	x1, x2
	b.eq	0b

	/* Clear jump address before jump */
	str	x2, [x0, #ACPI_PP_CPU_JMP_OFFSET]
	dsb	sy

	br	x1
ENDPROC(acpi_pp_code_start)
	/* Secondary Boot Code ends here */
__secondary_pp_code_end:
