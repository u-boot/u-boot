// SPDX-License-Identifier: GPL-2.0+
/*
 * K3: Architecture initialization
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <common.h>
#include <asm/io.h>
#include <spl.h>
#include <asm/arch/hardware.h>

#ifdef CONFIG_SPL_BUILD
static void store_boot_index_from_rom(void)
{
	u32 *boot_index = (u32 *)K3_BOOT_PARAM_TABLE_INDEX_VAL;

	*boot_index = *(u32 *)(CONFIG_SYS_K3_BOOT_PARAM_TABLE_INDEX);
}

void board_init_f(ulong dummy)
{
	/*
	 * Cannot delay this further as there is a chance that
	 * K3_BOOT_PARAM_TABLE_INDEX can be over written by SPL MALLOC section.
	 */
	store_boot_index_from_rom();

	/* Init DM early in-order to invoke system controller */
	spl_early_init();

	/* Prepare console output */
	preloader_console_init();
}

static u32 __get_backup_bootmedia(u32 devstat)
{
	u32 bkup_boot = (devstat & CTRLMMR_MAIN_DEVSTAT_BKUP_BOOTMODE_MASK) >>
			CTRLMMR_MAIN_DEVSTAT_BKUP_BOOTMODE_SHIFT;

	switch (bkup_boot) {
	case BACKUP_BOOT_DEVICE_USB:
		return BOOT_DEVICE_USB;
	case BACKUP_BOOT_DEVICE_UART:
		return BOOT_DEVICE_UART;
	case BACKUP_BOOT_DEVICE_ETHERNET:
		return BOOT_DEVICE_ETHERNET;
	case BACKUP_BOOT_DEVICE_MMC2:
		return BOOT_DEVICE_MMC2;
	case BACKUP_BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;
	case BACKUP_BOOT_DEVICE_HYPERFLASH:
		return BOOT_DEVICE_HYPERFLASH;
	case BACKUP_BOOT_DEVICE_I2C:
		return BOOT_DEVICE_I2C;
	};

	return BOOT_DEVICE_RAM;
}

static u32 __get_primary_bootmedia(u32 devstat)
{
	u32 bootmode = devstat & CTRLMMR_MAIN_DEVSTAT_BOOTMODE_MASK;

	if (bootmode == BOOT_DEVICE_OSPI || bootmode ==	BOOT_DEVICE_QSPI)
		bootmode = BOOT_DEVICE_SPI;

	return bootmode;
}

u32 spl_boot_device(void)
{
	u32 devstat = readl(CTRLMMR_MAIN_DEVSTAT);
	u32 bootindex = readl(K3_BOOT_PARAM_TABLE_INDEX_VAL);

	if (bootindex == K3_PRIMARY_BOOTMODE)
		return __get_primary_bootmedia(devstat);
	else
		return __get_backup_bootmedia(devstat);
}
#endif

#ifndef CONFIG_SYSRESET
void reset_cpu(ulong ignored)
{
}
#endif
