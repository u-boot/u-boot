/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Gateworks Corporation
 */

#ifndef __IMX8MM_VENICE_H
#define __IMX8MM_VENICE_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#define CONFIG_SPL_MAX_SIZE		(148 * 1024)
#define CONFIG_SYS_MONITOR_LEN		SZ_512K
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300
#define CONFIG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_STACK		0x920000
#define CONFIG_SPL_BSS_START_ADDR	0x910000
#define CONFIG_SPL_BSS_MAX_SIZE		SZ_8K
#define CONFIG_SYS_SPL_MALLOC_START	0x42200000
#define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_1M

/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CONFIG_MALLOC_F_ADDR		0x930000
/* For RAW image gives a error info not panic */
#define CONFIG_SPL_ABORT_ON_RAW_IMAGE

#endif

#define MEM_LAYOUT_ENV_SETTINGS \
	"fdt_addr_r=0x44000000\0" \
	"kernel_addr_r=0x42000000\0" \
	"ramdisk_addr_r=0x46400000\0" \
	"scriptaddr=0x46000000\0"

/* Link Definitions */
#define CONFIG_LOADADDR			0x40480000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

/* Enable Distro Boot */
#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#undef CONFIG_ISO_PARTITION
#else
#define BOOTENV
#endif

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS \
	BOOTENV \
	MEM_LAYOUT_ENV_SETTINGS \
	"script=boot.scr\0" \
	"bootm_size=0x10000000\0" \
	"ipaddr=192.168.1.22\0" \
	"serverip=192.168.1.146\0" \
	"dev=2\0" \
	"preboot=gsc wd-disable\0" \
	"console=ttymxc1,115200\0" \
	"update_firmware=" \
		"tftpboot $loadaddr $image && " \
		"setexpr blkcnt $filesize + 0x1ff && " \
		"setexpr blkcnt $blkcnt / 0x200 && " \
		"mmc dev $dev && " \
		"mmc write $loadaddr 0x42 $blkcnt\0" \
	"boot_net=" \
		"tftpboot $kernel_addr_r $image && " \
		"booti $kernel_addr_r - $fdtcontroladdr\0" \
	"update_rootfs=" \
		"tftpboot $loadaddr $image && " \
		"gzwrite mmc $dev $loadaddr $filesize 100000 1000000\0" \
	"update_all=" \
		"tftpboot $loadaddr $image && " \
		"gzwrite mmc $dev $loadaddr $filesize\0" \
	"erase_env=mmc dev $dev; mmc erase 0x7f08 0x40\0"

#define CONFIG_SYS_INIT_RAM_ADDR        0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE        SZ_2M
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		SZ_32M
#define CONFIG_SYS_SDRAM_BASE           0x40000000

/* SDRAM configuration */
#define PHYS_SDRAM                      0x40000000
#define PHYS_SDRAM_SIZE			SZ_1G /* 1GB DDR */
#define CONFIG_SYS_BOOTM_LEN		SZ_256M

/* UART */
#define CONFIG_MXC_UART_BASE		UART2_BASE_ADDR

/* Monitor Command Prompt */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_CBSIZE		SZ_2K
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

/* USDHC */
#define CONFIG_SYS_FSL_USDHC_NUM	2
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_MMC_IMG_LOAD_PART	1

/* I2C */
#define CONFIG_SYS_I2C_SPEED		100000

/* FEC */
#define CONFIG_ETHPRIME                 "eth0"
#define CONFIG_FEC_XCV_TYPE             RGMII
#define CONFIG_FEC_MXC_PHYADDR          0
#define FEC_QUIRK_ENET_MAC
#define IMX_FEC_BASE			0x30BE0000

#endif
