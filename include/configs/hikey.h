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

/* Physical Memory Map */

/* CONFIG_SYS_TEXT_BASE needs to align with where ATF loads bl33.bin */
#define CONFIG_SYS_TEXT_BASE		0x35000000

#define CONFIG_NR_DRAM_BANKS		6
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

#ifdef CONFIG_CMD_USB
#define CONFIG_USB_DWC2_REG_ADDR 0xF72C0000
/*#define CONFIG_DWC2_DFLT_SPEED_FULL*/
#define CONFIG_DWC2_ENABLE_DYNAMIC_FIFO

#define CONFIG_MISC_INIT_R
#endif

#define CONFIG_HIKEY_GPIO

/* SD/MMC configuration */
#define CONFIG_BOUNCE_BUFFER

#define CONFIG_FS_EXT4

/* Command line configuration */

#define CONFIG_MTD_PARTITIONS

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE

#include <config_distro_defaults.h>

/* Initial environment variables */

/*
 * Defines where the kernel and FDT will be put in RAM
 */

#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(MMC, mmc, 1) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS	\
				"kernel_name=Image\0"	\
				"kernel_addr_r=0x00080000\0" \
				"fdtfile=hi6220-hikey.dtb\0" \
				"fdt_addr_r=0x02000000\0" \
				"fdt_high=0xffffffffffffffff\0" \
				"initrd_high=0xffffffffffffffff\0" \
				BOOTENV

/* Preserve environment on sd card */
#define CONFIG_ENV_SIZE			0x1000
#define CONFIG_ENV_VARS_UBOOT_CONFIG

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_MAXARGS		64	/* max command args */

#endif /* __HIKEY_H */
