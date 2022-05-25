/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2022 Toradex
 */

#ifndef __VERDIN_IMX8MP_H
#define __VERDIN_IMX8MP_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#define CONFIG_SPL_MAX_SIZE				(152 * 1024)
#define CONFIG_SYS_MONITOR_LEN				SZ_512K
#define CONFIG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
/*#define CONFIG_ENABLE_DDR_TRAINING_DEBUG*/
#define CONFIG_SPL_STACK		0x960000
#define CONFIG_SPL_BSS_START_ADDR	0x0098fc00
#define CONFIG_SPL_BSS_MAX_SIZE		SZ_1K
#define CONFIG_SYS_SPL_MALLOC_START	0x42200000
#define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_512K

/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CONFIG_MALLOC_F_ADDR				0x184000
/* For RAW image gives a error info not panic */
#define CONFIG_SPL_ABORT_ON_RAW_IMAGE

#define CONFIG_POWER_PCA9450

#define CONFIG_SYS_I2C
#endif /* CONFIG_SPL_BUILD */

/* ENET Config */
/* ENET1 */
#if defined(CONFIG_CMD_NET)
#define CONFIG_FEC_MXC_PHYADDR		7

#define PHY_ANEG_TIMEOUT 20000
#endif /* CONFIG_CMD_NET */

#define MEM_LAYOUT_ENV_SETTINGS \
	"fdt_addr_r=0x43000000\0" \
	"kernel_addr_r=0x40000000\0" \
	"ramdisk_addr_r=0x46400000\0" \
	"scriptaddr=0x46000000\0"

/* Enable Distro Boot */
#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#else
#define BOOTENV
#endif

#if defined(CONFIG_TDX_EASY_INSTALLER)
#  define BOOT_SCRIPT	"boot-tezi.scr"
#else
#  define BOOT_SCRIPT	"boot.scr"
#endif

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS \
	BOOTENV \
	MEM_LAYOUT_ENV_SETTINGS \
	"bootcmd_mfg=fastboot 0\0" \
	"boot_file=Image\0" \
	"boot_scripts=" BOOT_SCRIPT "\0" \
	"boot_script_dhcp=" BOOT_SCRIPT "\0" \
	"console=ttymxc2\0" \
	"fdt_board=dev\0" \
	"initrd_addr=0x43800000\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"setup=setenv setupargs console=tty1 console=${console},${baudrate} " \
		"consoleblank=0 earlycon\0" \
	"update_uboot=askenv confirm Did you load flash.bin (y/N)?; " \
		"if test \"$confirm\" = \"y\"; then " \
		"setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt " \
		"${blkcnt} / 0x200; mmc dev 2 1; mmc write ${loadaddr} 0x0 " \
		"${blkcnt}; fi\0"

#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE	SZ_512K
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_BOOTM_LEN		SZ_64M /* Increase max gunzip size */

/* i.MX 8M Plus supports max. 8GB memory in two albeit concecutive banks */
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			(SZ_2G + SZ_1G)
#define PHYS_SDRAM_2			0x100000000
#define PHYS_SDRAM_2_SIZE		(SZ_4G + SZ_1G)

/* UART */
#define CONFIG_MXC_UART_BASE		UART_BASE_ADDR(3)

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		SZ_2K
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

#endif /* __VERDIN_IMX8MP_H */
