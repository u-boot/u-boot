/*
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_DB_88F6820_GP_H
#define _CONFIG_DB_88F6820_GP_H

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_ARMADA_XP		/* SOC Family Name */
#define CONFIG_DB_88F6820_GP		/* Board target name for DDR training */

#define CONFIG_SYS_L2_PL310

#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */
#define CONFIG_SYS_GENERIC_BOARD
#define CONFIG_DISPLAY_BOARDINFO_LATE

#define	CONFIG_SYS_TEXT_BASE	0x04000000
#define CONFIG_SYS_TCLK		250000000	/* 250MHz */

/*
 * Commands configuration
 */
#define CONFIG_SYS_NO_FLASH		/* Declare no flash (NOR/SPI) */
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ENV
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_FAT
#define CONFIG_CMD_FS_GENERIC
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MMC
#define CONFIG_CMD_PING
#define CONFIG_CMD_SCSI
#define CONFIG_CMD_SF
#define CONFIG_CMD_SPI
#define CONFIG_CMD_TFTPPUT
#define CONFIG_CMD_TIME
#define CONFIG_CMD_USB

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

/*
 * SATA/SCSI/AHCI configuration
 */
#define CONFIG_LIBATA
#define CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	2
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					 CONFIG_SYS_SCSI_MAX_LUN)

/* Partition support */
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION

/* Additional FS support/configuration */
#define CONFIG_SUPPORT_VFAT

/* USB/EHCI configuration */
#define CONFIG_USB_EHCI
#define CONFIG_USB_STORAGE
#define CONFIG_USB_EHCI_MARVELL
#define CONFIG_EHCI_IS_TDI

/* Environment in SPI NOR flash */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET		(1 << 20) /* 1MiB in */
#define CONFIG_ENV_SIZE			(64 << 10) /* 64KiB */
#define CONFIG_ENV_SECT_SIZE		(256 << 10) /* 256KiB sectors */

#define CONFIG_PHY_MARVELL		/* there is a marvell phy */
#define CONFIG_PHY_ADDR			{ 1, 0 }
#define CONFIG_SYS_NETA_INTERFACE_TYPE	PHY_INTERFACE_MODE_RGMII
#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

#define CONFIG_SYS_CONSOLE_INFO_QUIET	/* don't print console @ startup */
#define CONFIG_SYS_ALT_MEMTEST

/* Keep device tree and initrd in lower memory so the kernel can access them */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0"

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

#endif /* _CONFIG_DB_88F6820_GP_H */
