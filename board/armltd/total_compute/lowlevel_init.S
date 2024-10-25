/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2024 Arm Limited
 */

.global save_boot_params
save_boot_params:
	/* The firmware provided FDT address via x1 */
	adr	x8, fw_dtb_pointer
	str	x1, [x8]

	b	save_boot_params_ret
