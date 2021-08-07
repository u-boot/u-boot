/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 NXP
 */

#include <config.h>

.align 8
.global rom_pointer
rom_pointer:
	.space 256

/*
 * Routine: save_boot_params (called after reset from start.S)
 */

.global save_boot_params
save_boot_params:
	/* The firmware provided ATAG/FDT address can be found in r2/x0 */
	adr	x0, rom_pointer
	stp	x1, x2, [x0], #16
	stp	x3, x4, [x0], #16

	/* Returns */
	b	save_boot_params_ret

.global restore_boot_params
restore_boot_params:
	adr	x0, rom_pointer
	ldp	x1, x2, [x0], #16
	ldp	x3, x4, [x0], #16
	ret
