/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __OCTEONTX_COMMON_H__
#define __OCTEONTX_COMMON_H__

#ifdef CONFIG_DISTRO_DEFAULTS
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(USB, usb, 0) \
	func(SCSI, scsi, 0)

#include <config_distro_bootcmd.h>
/* Extra environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"autoload=0\0"			\
	"loadaddr=0x20080000\0"		\
	"kernel_addr_r=0x02000000\0"	\
	"ramdisk_addr_r=0x03000000\0"	\
	"scriptaddr=0x04000000\0"	\
	BOOTENV

#else

/** Extra environment settings */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"loadaddr=20080000\0"		\
	"autoload=0\0"

#endif /* ifdef CONFIG_DISTRO_DEFAULTS*/

/** Maximum size of image supported for bootm (and bootable FIT images) */
#define CONFIG_SYS_BOOTM_LEN		(256 << 20)

/** Memory base address */
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_TEXT_BASE

/** Stack starting address */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0xffff0)

/** Heap size for U-Boot */

/* Autoboot options */
#define CONFIG_RESET_TO_RETRY
#define CONFIG_BOOT_RETRY_TIME		-1
#define CONFIG_BOOT_RETRY_MIN		30

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE

/* AHCI support Definitions */
#ifdef CONFIG_DM_SCSI
/** Enable 48-bit SATA addressing */
# define CONFIG_LBA48
/** Enable 64-bit addressing */
# define CONFIG_SYS_64BIT_LBA
#endif

/***** SPI Defines *********/
#ifdef CONFIG_DM_SPI_FLASH
# define CONFIG_SF_DEFAULT_BUS	0
# define CONFIG_SF_DEFAULT_CS	0
#endif

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		1024	/** Console I/O Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MAXARGS		64	/** max command args */

#define CONFIG_SYS_MMC_MAX_BLK_COUNT	8192

/** EMMC specific defines */
#if defined(CONFIG_MMC_OCTEONTX)
#define CONFIG_SUPPORT_EMMC_RPMB
#endif

#if defined(CONFIG_NAND_OCTEONTX)
/*#define CONFIG_MTD_CONCAT */
#define CONFIG_SYS_MAX_NAND_DEVICE 8
#endif

#endif /* __OCTEONTX_COMMON_H__ */
