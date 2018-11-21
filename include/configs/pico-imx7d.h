/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 NXP Semiconductors
 *
 * Configuration settings for the i.MX7D Pico board.
 */

#ifndef __PICO_IMX7D_CONFIG_H
#define __PICO_IMX7D_CONFIG_H

#include "mx7_common.h"

#include "imx7_spl.h"

#ifdef CONFIG_SPL_OS_BOOT
/* Falcon Mode */
#define CONFIG_SPL_FS_LOAD_ARGS_NAME	"args"
#define CONFIG_SPL_FS_LOAD_KERNEL_NAME	"uImage"
#define CONFIG_SYS_SPL_ARGS_ADDR	0x88000000

/* Falcon Mode - MMC support: args@1MB kernel@2MB */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR  0x800   /* 1MB */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS (CONFIG_CMD_SPL_WRITE_SIZE / 512)
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR        0x1000  /* 2MB */
#endif

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(32 * SZ_1M)

#define CONFIG_MXC_UART_BASE		UART5_IPS_BASE_ADDR

/* Network */
#define CONFIG_FEC_MXC
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		1

#define CONFIG_PHY_ATHEROS

/* ENET1 */
#define IMX_FEC_BASE			ENET_IPS_BASE_ADDR

/* MMC Config */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0

#define CONFIG_DFU_ENV_SETTINGS \
	"dfu_alt_info=" \
		"spl raw 0x2 0x400;" \
		"u-boot raw 0x8a 0x400;" \
		"/boot/zImage ext4 0 1;" \
		"/boot/imx7d-pico-hobbit.dtb ext4 0 1;" \
		"/boot/imx7d-pico-pi.dtb ext4 0 1;" \
		"rootfs part 0 1\0" \

#define BOOTMENU_ENV \
	"bootmenu_0=Boot using PICO-Hobbit baseboard=" \
		"setenv fdtfile imx7d-pico-hobbit.dtb\0" \
	"bootmenu_1=Boot using PICO-Pi baseboard=" \
		"setenv fdtfile imx7d-pico-pi.dtb\0" \

#define CONFIG_SUPPORT_EMMC_BOOT /* eMMC specific */
#define CONFIG_SYS_MMC_IMG_LOAD_PART	1

#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"image=zImage\0" \
	"console=ttymxc4\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	BOOTMENU_ENV \
	"fdt_addr=0x83000000\0" \
	"fdt_addr_r=0x83000000\0" \
	"kernel_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	"pxefile_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	"ramdisk_addr_r=0x83000000\0" \
	"ramdiskaddr=0x83000000\0" \
	"scriptaddr=" __stringify(CONFIG_LOADADDR) "\0" \
	CONFIG_DFU_ENV_SETTINGS \
	"findfdt=" \
		"if test $fdtfile = ask ; then " \
			"bootmenu -1; fi;" \
		"if test $fdtfile != ask ; then " \
			"saveenv; fi;\0" \
	"finduuid=part uuid mmc 0:1 uuid\0" \
	"partitions=" \
		"uuid_disk=${uuid_gpt_disk};" \
		"name=rootfs,size=0,uuid=${uuid_gpt_rootfs}\0" \
	"fastboot_partition_alias_system=rootfs\0" \
	"setup_emmc=mmc dev 0; gpt write mmc 0 $partitions; reset;\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x20000000)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* I2C configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1
#define CONFIG_SYS_I2C_MXC_I2C2
#define CONFIG_SYS_I2C_MXC_I2C3
#define CONFIG_SYS_I2C_MXC_I2C4
#define CONFIG_SYS_I2C_SPEED		100000

/* PMIC */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE3000
#define CONFIG_POWER_PFUZE3000_I2C_ADDR	0x08

/* FLASH and environment organization */
#define CONFIG_ENV_SIZE			SZ_8K

#define CONFIG_ENV_OFFSET			(8 * SZ_64K)
#define CONFIG_SYS_FSL_USDHC_NUM		2

#define CONFIG_SYS_MMC_ENV_DEV			0
#define CONFIG_SYS_MMC_ENV_PART		0

/* USB Configs */
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC			(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS			0
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2

#define CONFIG_IMX_THERMAL

#endif
