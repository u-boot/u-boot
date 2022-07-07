/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 Gateworks Corporation
 */

#ifndef __IMX8MM_VENICE_H
#define __IMX8MM_VENICE_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#define CONFIG_SYS_MONITOR_LEN		SZ_512K
#define CONFIG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
/* For RAW image gives a error info not panic */
#endif

#define MEM_LAYOUT_ENV_SETTINGS \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"fdt_addr_r=0x50200000\0" \
	"scriptaddr=0x50280000\0" \
	"ramdisk_addr_r=0x50300000\0" \
	"kernel_comp_addr_r=0x40200000\0"

/* Enable Distro Boot */
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS \
	BOOTENV \
	MEM_LAYOUT_ENV_SETTINGS \
	"script=boot.scr\0" \
	"bootm_size=0x10000000\0" \
	"dev=2\0" \
	"preboot=gsc wd-disable\0" \
	"console=ttymxc1,115200\0" \
	"update_firmware=" \
		"tftpboot $loadaddr $image && " \
		"setexpr blkcnt $filesize + 0x1ff && " \
		"setexpr blkcnt $blkcnt / 0x200 && " \
		"mmc dev $dev && " \
		"mmc write $loadaddr 0x40 $blkcnt\0" \
	"loadfdt=" \
		"if $fsload $fdt_addr_r $dir/$fdt_file1; " \
			"then echo loaded $fdt_file1; " \
		"elif $fsload $fdt_addr_r $dir/$fdt_file2; " \
			"then echo loaded $fdt_file2; " \
		"elif $fsload $fdt_addr_r $dir/$fdt_file3; " \
			"then echo loaded $fdt_file3; " \
		"elif $fsload $fdt_addr_r $dir/$fdt_file4; " \
			"then echo loaded $fdt_file4; " \
		"elif $fsload $fdt_addr_r $dir/$fdt_file5; " \
			"then echo loaded $fdt_file5; " \
		"fi\0" \
	"boot_net=" \
		"setenv fsload tftpboot; " \
		"run loadfdt && tftpboot $kernel_addr_r $dir/Image && " \
		"booti $kernel_addr_r - $fdt_addr_r\0" \
	"update_rootfs=" \
		"tftpboot $loadaddr $image && " \
		"gzwrite mmc $dev $loadaddr $filesize 100000 1000000\0" \
	"update_all=" \
		"tftpboot $loadaddr $image && " \
		"gzwrite mmc $dev $loadaddr $filesize\0" \
	"erase_env=mmc dev $dev; mmc erase 0x7f08 0x40\0"

#define CONFIG_SYS_INIT_RAM_ADDR        0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE        SZ_2M

#define CONFIG_SYS_SDRAM_BASE           0x40000000

/* SDRAM configuration */
#define PHYS_SDRAM                      0x40000000
#define PHYS_SDRAM_SIZE			SZ_4G

/* FEC */
#define CONFIG_FEC_MXC_PHYADDR          0
#define FEC_QUIRK_ENET_MAC

#endif
