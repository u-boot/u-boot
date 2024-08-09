/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 9elements GmbH
 *   Author: Patrick Rudolph <patrick.rudolph@9elements.com>
 */

#include <linux/linkage.h>
#include <asm/acpi_table.h>

__secondary_cpuif_id:
	.quad 0

__secondary_parking_protocol_tables:
	.quad 0

__secondary_parking_protocol_ncpus:
	.quad 0

.global acpi_parking_protocol_code_size
acpi_parking_protocol_code_size:
	.word __secondary_parking_protocol_code_end - __secondary_parking_protocol_code_start

.global acpi_parking_protocol_install
ENTRY(acpi_parking_protocol_install)
	ldr	x2, =__secondary_parking_protocol_ncpus
	str	x1, [x2]
	ldr	x2, =__secondary_parking_protocol_tables
	str	x0, [x2]
	dsb	ishst
	sev
	ret
ENDPROC(acpi_parking_protocol_install)

.global acpi_parking_protocol_secondary_jump
ENTRY(acpi_parking_protocol_secondary_jump)
0:
	/* Store a unique cpuif id in x0 */
	ldr	x2, =__secondary_cpuif_id
	ldaxr	w0, [x2]
	add	w0, w0, #1  // =1
	stlxr	w1, w0, [x2]
	cbnz	w1, 0b

0:	/* Check if parking protocol table is ready */
	ldr	x1, =__secondary_parking_protocol_tables
	ldr	x2, [x1]
	cbnz	x2, 0f
	wfe
	b	0b

0:	/* Sanity check cpuif id */
	ldr	x1, =__secondary_parking_protocol_ncpus
	ldr	x3, [x1]
	cmp	w0, w3
	b.lt	0f

hlt:	wfi
	b	hlt	/* Should never happen. */

	/* Find spinning code in ACPI parking protocol table */
0:	mov	x1, #ACPI_PP_PAGE_SIZE
	mul	x3, x0, x1
	add	x2, x2, x3
	mov	x0, x2		/* Backup ACPI page ptr for later use */

	mov	x1, #ACPI_PP_CPU_CODE_OFFSET
	add	x2, x2, x1

	/* Jump to spin code in own parking protocol page */
	br	x2
ENDPROC(acpi_parking_protocol_secondary_jump)

.align 8
__secondary_parking_protocol_code_start:
.global acpi_parking_protocol_code_start
ENTRY(acpi_parking_protocol_code_start)
	/* x0 points to the 4K aligned parking protocol page */

	/* Prepare defines for spinning code */
	mov	w3, #ACPI_PP_CPU_ID_INVALID
	mov	x2, #ACPI_PP_JMP_ADR_INVALID

0:	wfe
	ldr	w1, [x0, #ACPI_PP_CPU_ID_OFFSET]

	/* Check CPU ID is not invalid */
	cmp	w1, w3
	b.eq	0b

	/* Check jump address not invalid */
	ldr	x1, [x0, #ACPI_PP_CPU_JMP_ADDR_OFFSET]
	cmp	x1, x2
	b.eq	0b

	/* Clear jump address before jump */
	str	x2, [x0, #ACPI_PP_CPU_JMP_ADDR_OFFSET]
	dsb	sy

	br	x1
ENDPROC(acpi_parking_protocol_code_start)
	/* Secondary Boot Code ends here */
__secondary_parking_protocol_code_end:
