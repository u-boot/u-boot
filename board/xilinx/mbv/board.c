// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2023, Advanced Micro Devices, Inc.
 *
 * Michal Simek <michal.simek@amd.com>
 */

#include <spl.h>

int board_init(void)
{
	return 0;
}

#ifdef CONFIG_SPL
u32 spl_boot_device(void)
{
	/* RISC-V QEMU only supports RAM as SPL boot device */
	return BOOT_DEVICE_RAM;
}
#endif
