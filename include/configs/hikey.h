/*
 * (C) Copyright 2015 Linaro
 *
 * Peter Griffin <peter.griffin@linaro.org>
 *
 * Configuration for HiKey 96boards CE. Parts were derived from other ARM
 * configurations.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __HIKEY_H
#define __HIKEY_H

#include <linux/sizes.h>

#define CONFIG_POWER
#define CONFIG_POWER_HI6553

#define CONFIG_REMAKE_ELF

#define CONFIG_SUPPORT_RAW_INITRD

/* Cache Definitions */
#define CONFIG_SYS_DCACHE_OFF

#define CONFIG_IDENT_STRING		"hikey"

/* Flat Device Tree Definitions */
#define CONFIG_OF_LIBFDT

#define CONFIG_BOARD_EARLY_INIT_F

/* Physical Memory Map */

/* CONFIG_SYS_TEXT_BASE needs to align with where ATF loads bl33.bin */
#define CONFIG_SYS_TEXT_BASE		0x35000000

#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			0x00000000

/* 1008 MB (the last 16Mb are secured for TrustZone by ATF*/
#define PHYS_SDRAM_1_SIZE		0x3EFFFFFF

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

#define CONFIG_SYS_INIT_RAM_SIZE	0x1000

#define CONFIG_SYS_INIT_SP_ADDR         (CONFIG_SYS_SDRAM_BASE + 0x7fff0)

#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x80000)

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		19000000

/* Generic Interrupt Controller Definitions */
#define GICD_BASE			0xf6801000
#define GICC_BASE			0xf6802000

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + SZ_8M)

/* Serial port PL010/PL011 through the device model */
#define CONFIG_PL01X_SERIAL
#define CONFIG_BAUDRATE			115200

#define CONFIG_CMD_USB
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_DWC2
#define CONFIG_USB_DWC2_REG_ADDR 0xF72C0000
/*#define CONFIG_DWC2_DFLT_SPEED_FULL*/
#define CONFIG_DWC2_ENABLE_DYNAMIC_FIFO

#define CONFIG_USB_STORAGE
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_SMSC95XX
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_MISC_INIT_R
#endif

#define CONFIG_HIKEY_GPIO

/* SD/MMC configuration */
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_DWMMC
#define CONFIG_HIKEY_DWMMC
#define CONFIG_BOUNCE_BUFFER
#define CONFIG_CMD_MMC

#define CONFIG_FS_EXT4

/* Command line configuration */
#define CONFIG_MENU
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_UNZIP
#define CONFIG_CMD_ENV

#define CONFIG_MTD_PARTITIONS

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE

#include <config_distro_defaults.h>

/* Initial environment variables */

/*
 * Defines where the kernel and FDT will be put in RAM
 */

/* Assume we boot with root on the seventh partition of eMMC */
#define CONFIG_BOOTARGS	"console=ttyAMA0,115200n8 root=/dev/mmcblk0p9 rw"

#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(MMC, mmc, 1) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS	\
				"kernel_name=Image\0"	\
				"kernel_addr_r=0x00080000\0" \
				"fdt_name=hi6220-hikey.dtb\0" \
				"fdt_addr_r=0x02000000\0" \
				"fdt_high=0xffffffffffffffff\0" \
				"initrd_high=0xffffffffffffffff\0" \
				BOOTENV


/* Preserve enviroment on sd card */
#define CONFIG_COMMAND_HISTORY

#define CONFIG_ENV_SIZE			0x1000
#define CONFIG_ENV_IS_IN_FAT
#define FAT_ENV_INTERFACE               "mmc"
#define FAT_ENV_DEVICE_AND_PART         "1:1"
#define FAT_ENV_FILE                    "uboot.env"
#define CONFIG_FAT_WRITE
#define CONFIG_ENV_VARS_UBOOT_CONFIG

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_MAXARGS		64	/* max command args */

#define CONFIG_SYS_NO_FLASH

#endif /* __HIKEY_H */
