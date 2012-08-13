/*
 * boot-common.c
 *
 * Common bootmode functions for omap based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <spl.h>
#include <asm/omap_common.h>
#include <asm/arch/omap.h>
#include <asm/arch/mmc_host_def.h>

/*
 * This is used to verify if the configuration header
 * was executed by rom code prior to control of transfer
 * to the bootloader. SPL is responsible for saving and
 * passing the boot_params pointer to the u-boot.
 */
struct omap_boot_parameters boot_params __attribute__ ((section(".data")));

#ifdef CONFIG_SPL_BUILD
/*
 * We use static variables because global data is not ready yet.
 * Initialized data is available in SPL right from the beginning.
 * We would not typically need to save these parameters in regular
 * U-Boot. This is needed only in SPL at the moment.
 */
u32 omap_bootmode = MMCSD_MODE_FAT;

u32 spl_boot_device(void)
{
	return (u32) (boot_params.omap_bootdevice);
}

u32 spl_boot_mode(void)
{
	return omap_bootmode;
}

void spl_board_init(void)
{
#ifdef CONFIG_SPL_NAND_SUPPORT
	gpmc_init();
#endif
}

int board_mmc_init(bd_t *bis)
{
	switch (spl_boot_device()) {
	case BOOT_DEVICE_MMC1:
		omap_mmc_init(0, 0, 0);
		break;
	case BOOT_DEVICE_MMC2:
	case BOOT_DEVICE_MMC2_2:
		omap_mmc_init(1, 0, 0);
		break;
	}
	return 0;
}
#endif
