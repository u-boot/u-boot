// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2023-2025, Advanced Micro Devices, Inc.
 *
 * Michal Simek <michal.simek@amd.com>
 */

#include <spl.h>

#ifdef CONFIG_SPL
void board_boot_order(u32 *spl_boot_list)
{
	u32 i = 0;

	if (CONFIG_IS_ENABLED(SPI_FLASH_SUPPORT))
		spl_boot_list[i++] = BOOT_DEVICE_SPI;

	if (CONFIG_IS_ENABLED(RAM_SUPPORT))
		spl_boot_list[i++] = BOOT_DEVICE_RAM;

}
#endif
