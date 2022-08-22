/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2022 Toradex
 */

#ifndef __VERDIN_IMX8MP_H
#define __VERDIN_IMX8MP_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#define CONFIG_SYS_MONITOR_LEN				SZ_512K
#define CONFIG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
/*#define CONFIG_ENABLE_DDR_TRAINING_DEBUG*/

/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CONFIG_MALLOC_F_ADDR				0x184000
/* For RAW image gives a error info not panic */

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
	"fdt_addr_r=0x50200000\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernel_comp_addr_r=0x40200000\0" \
	"kernel_comp_size=0x08080000\0" \
	"ramdisk_addr_r=0x50300000\0" \
	"scriptaddr=0x50280000\0"

/* Enable Distro Boot */
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

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

/* i.MX 8M Plus supports max. 8GB memory in two albeit concecutive banks */
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			(SZ_2G + SZ_1G)
#define PHYS_SDRAM_2			0x100000000
#define PHYS_SDRAM_2_SIZE		(SZ_4G + SZ_1G)

#endif /* __VERDIN_IMX8MP_H */
