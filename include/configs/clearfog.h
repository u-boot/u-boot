/*
 * Copyright (C) 2015 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_CLEARFOG_H
#define _CONFIG_CLEARFOG_H

/*
 * High Level Configuration Options (easy to change)
 */

#define CONFIG_DISPLAY_BOARDINFO_LATE

/*
 * TEXT_BASE needs to be below 16MiB, since this area is scrubbed
 * for DDR ECC byte filling in the SPL before loading the main
 * U-Boot into it.
 */
#define	CONFIG_SYS_TEXT_BASE	0x00800000
#define CONFIG_SYS_TCLK		250000000	/* 250MHz */

/*
 * Commands configuration
 */
#define CONFIG_SYS_NO_FLASH		/* Declare no flash (NOR/SPI) */
#define CONFIG_CMD_ENV
#define CONFIG_CMD_PCI

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MVTWSI
#define CONFIG_I2C_MVTWSI_BASE0		MVEBU_TWSI_BASE
#define CONFIG_SYS_I2C_SLAVE		0x0
#define CONFIG_SYS_I2C_SPEED		100000

/* SPI NOR flash default params, used by sf commands */
#define CONFIG_SF_DEFAULT_SPEED		1000000
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_3
#define CONFIG_SPI_FLASH_STMICRO

/*
 * SDIO/MMC Card Configuration
 */
#define CONFIG_MMC
#define CONFIG_MMC_SDMA
#define CONFIG_GENERIC_MMC
#define CONFIG_SDHCI
#define CONFIG_MV_SDHCI
#define CONFIG_SYS_MMC_BASE		MVEBU_SDIO_BASE

/* Partition support */
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION

/* Additional FS support/configuration */
#define CONFIG_SUPPORT_VFAT

/* USB/EHCI configuration */
#define CONFIG_EHCI_IS_TDI

#define CONFIG_ENV_MIN_ENTRIES		128

/* Environment in MMC */
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_SECT_SIZE		0x200
#define CONFIG_ENV_SIZE			0x10000
/*
 * For SD - reserve 1 LBA for MBR + 1M for u-boot image. The MMC/eMMC
 * boot image starts @ LBA-0.
 * As result in MMC/eMMC case it will be a 1 sector gap between u-boot
 * image and environment
 */
#define CONFIG_ENV_OFFSET		0xf0000
#define CONFIG_ENV_ADDR			CONFIG_ENV_OFFSET

#define CONFIG_PHY_MARVELL		/* there is a marvell phy */
#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

/* PCIe support */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_PCI
#define CONFIG_PCI_MVEBU
#define CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW
#endif

#define CONFIG_SYS_CONSOLE_INFO_QUIET	/* don't print console @ startup */
#define CONFIG_SYS_ALT_MEMTEST

/* Keep device tree and initrd in lower memory so the kernel can access them */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0"

/* SPL */
/*
 * Select the boot device here
 *
 * Currently supported are:
 * SPL_BOOT_SPI_NOR_FLASH	- Booting via SPI NOR flash
 * SPL_BOOT_SDIO_MMC_CARD	- Booting via SDIO/MMC card (partition 1)
 */
#define SPL_BOOT_SPI_NOR_FLASH		1
#define SPL_BOOT_SDIO_MMC_CARD		2
#define CONFIG_SPL_BOOT_DEVICE		SPL_BOOT_SDIO_MMC_CARD

/* Defines for SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_SIZE			(140 << 10)
#define CONFIG_SPL_TEXT_BASE		0x40000030
#define CONFIG_SPL_MAX_SIZE		(CONFIG_SPL_SIZE - 0x0030)

#define CONFIG_SPL_BSS_START_ADDR	(0x40000000 + CONFIG_SPL_SIZE)
#define CONFIG_SPL_BSS_MAX_SIZE		(16 << 10)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MALLOC_SIMPLE
#endif

#define CONFIG_SPL_STACK		(0x40000000 + ((192 - 16) << 10))
#define CONFIG_SPL_BOOTROM_SAVE		(CONFIG_SPL_STACK + 4)

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT

#if CONFIG_SPL_BOOT_DEVICE == SPL_BOOT_SPI_NOR_FLASH
/* SPL related SPI defines */
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000
#define CONFIG_SYS_U_BOOT_OFFS		CONFIG_SYS_SPI_U_BOOT_OFFS
#endif

#if CONFIG_SPL_BOOT_DEVICE == SPL_BOOT_SDIO_MMC_CARD
/* SPL related MMC defines */
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SYS_MMC_U_BOOT_OFFS		(160 << 10)
#define CONFIG_SYS_U_BOOT_OFFS			CONFIG_SYS_MMC_U_BOOT_OFFS
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	((CONFIG_SYS_U_BOOT_OFFS / 512)\
						 + 1)
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	((512 << 10) / 512) /* 512KiB */
#ifdef CONFIG_SPL_BUILD
#define CONFIG_FIXED_SDHCI_ALIGNED_BUFFER	0x00180000	/* in SDRAM */
#endif
#endif

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

#endif /* _CONFIG_CLEARFOG_H */
