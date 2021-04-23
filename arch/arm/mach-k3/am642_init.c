// SPDX-License-Identifier: GPL-2.0
/*
 * AM642: SoC specific initialization
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 *	Keerthy <j-keerthy@ti.com>
 *	Dave Gerlach <d-gerlach@ti.com>
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include "common.h"

#if defined(CONFIG_SPL_BUILD)

static void ctrl_mmr_unlock(void)
{
	/* Unlock all PADCFG_MMR1 module registers */
	mmr_unlock(PADCFG_MMR1_BASE, 1);

	/* Unlock all CTRL_MMR0 module registers */
	mmr_unlock(CTRL_MMR0_BASE, 0);
	mmr_unlock(CTRL_MMR0_BASE, 1);
	mmr_unlock(CTRL_MMR0_BASE, 2);
	mmr_unlock(CTRL_MMR0_BASE, 3);
	mmr_unlock(CTRL_MMR0_BASE, 5);
	mmr_unlock(CTRL_MMR0_BASE, 6);
}

/*
 * This uninitialized global variable would normal end up in the .bss section,
 * but the .bss is cleared between writing and reading this variable, so move
 * it to the .data section.
 */
u32 bootindex __section(".data");
static struct rom_extended_boot_data bootdata __section(.data);

static void store_boot_info_from_rom(void)
{
	bootindex = *(u32 *)(CONFIG_SYS_K3_BOOT_PARAM_TABLE_INDEX);
	memcpy(&bootdata, (uintptr_t *)ROM_ENTENDED_BOOT_DATA_INFO,
	       sizeof(struct rom_extended_boot_data));
}

void board_init_f(ulong dummy)
{
#if defined(CONFIG_CPU_V7R)
	setup_k3_mpu_regions();
#endif

	/*
	 * Cannot delay this further as there is a chance that
	 * K3_BOOT_PARAM_TABLE_INDEX can be over written by SPL MALLOC section.
	 */
	store_boot_info_from_rom();

	ctrl_mmr_unlock();

	/* Init DM early */
	spl_early_init();

	preloader_console_init();
}

u32 spl_boot_mode(const u32 boot_device)
{
	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		return MMCSD_MODE_EMMCBOOT;

	case BOOT_DEVICE_MMC2:
		return MMCSD_MODE_FS;

	default:
		return MMCSD_MODE_RAW;
	}
}

static u32 __get_backup_bootmedia(u32 main_devstat)
{
	u32 bkup_bootmode =
	    (main_devstat & MAIN_DEVSTAT_BACKUP_BOOTMODE_MASK) >>
	    MAIN_DEVSTAT_BACKUP_BOOTMODE_SHIFT;
	u32 bkup_bootmode_cfg =
	    (main_devstat & MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_MASK) >>
	    MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_SHIFT;

	switch (bkup_bootmode) {
	case BACKUP_BOOT_DEVICE_UART:
		return BOOT_DEVICE_UART;

	case BACKUP_BOOT_DEVICE_USB:
		return BOOT_DEVICE_USB;

	case BACKUP_BOOT_DEVICE_ETHERNET:
		return BOOT_DEVICE_ETHERNET;

	case BACKUP_BOOT_DEVICE_MMC:
		if (bkup_bootmode_cfg)
			return BOOT_DEVICE_MMC2;
		return BOOT_DEVICE_MMC1;

	case BACKUP_BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;

	case BACKUP_BOOT_DEVICE_I2C:
		return BOOT_DEVICE_I2C;
	};

	return BOOT_DEVICE_RAM;
}

static u32 __get_primary_bootmedia(u32 main_devstat)
{
	u32 bootmode = (main_devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_MASK) >>
	    MAIN_DEVSTAT_PRIMARY_BOOTMODE_SHIFT;
	u32 bootmode_cfg =
	    (main_devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_MASK) >>
	    MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_SHIFT;

	switch (bootmode) {
	case BOOT_DEVICE_OSPI:
		fallthrough;
	case BOOT_DEVICE_QSPI:
		fallthrough;
	case BOOT_DEVICE_XSPI:
		fallthrough;
	case BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;

	case BOOT_DEVICE_ETHERNET_RGMII:
		fallthrough;
	case BOOT_DEVICE_ETHERNET_RMII:
		return BOOT_DEVICE_ETHERNET;

	case BOOT_DEVICE_EMMC:
		return BOOT_DEVICE_MMC1;

	case BOOT_DEVICE_MMC:
		if ((bootmode_cfg & MAIN_DEVSTAT_PRIMARY_MMC_PORT_MASK) >>
		     MAIN_DEVSTAT_PRIMARY_MMC_PORT_SHIFT)
			return BOOT_DEVICE_MMC2;
		return BOOT_DEVICE_MMC1;

	case BOOT_DEVICE_NOBOOT:
		return BOOT_DEVICE_RAM;
	}

	return bootmode;
}

u32 spl_boot_device(void)
{
	u32 devstat = readl(CTRLMMR_MAIN_DEVSTAT);

	if (bootindex == K3_PRIMARY_BOOTMODE)
		return __get_primary_bootmedia(devstat);
	else
		return __get_backup_bootmedia(devstat);
}
#endif
