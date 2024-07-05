/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "tegra30-common.h"

/* High-level configuration options */
#define CFG_TEGRA_BOARD_STRING		"Lenovo Ideapad Yoga 11"

#define IDEAPAD_FLASH_UBOOT \
	"flash_uboot=sf probe 0:1;" \
		"echo Dumping current SPI flash content ...;" \
		"sf read ${kernel_addr_r} 0x0 ${spi_size};" \
		"if fatwrite mmc 1:1 ${kernel_addr_r} spi-flash-backup.bin ${spi_size};" \
		"then echo SPI flash content was successfully written into spi-flash-backup.bin;" \
			"echo Reading SPI flash binary;" \
			"if load mmc 1:1 ${kernel_addr_r} repart-block.bin;" \
			"then echo Writing bootloader into SPI flash;" \
				"sf probe 0:1;" \
				"sf update ${kernel_addr_r} 0x0 ${spi_size};" \
				"echo Bootloader SUCCESSFULLY written into SPI flash;" \
				"pause 'Press ANY key to reboot...'; reset;" \
			"else echo Preparing RAM;" \
				"mw ${kernel_addr_r} 0 ${boot_block_size_r};" \
				"mw ${ramdisk_addr_r} 0 ${boot_block_size_r};" \
				"echo Reading BCT;" \
				"sf read ${kernel_addr_r} 0x0 ${boot_block_size_r};" \
				"echo Reading bootloader;" \
				"if load mmc 1:1 ${ramdisk_addr_r} ${bootloader_file};" \
				"then echo Calculating bootloader size;" \
					"size mmc 1:1 ${bootloader_file};" \
					"ebtupdate ${kernel_addr_r} ${ramdisk_addr_r} ${filesize};" \
					"echo Writing bootloader into SPI flash;" \
					"sf probe 0:1;" \
					"sf update ${kernel_addr_r} 0x0 ${boot_block_size_r};" \
					"sf update ${ramdisk_addr_r} ${boot_block_size_r} ${boot_block_size_r};" \
					"echo Bootloader written SUCCESSFULLY;" \
					"pause 'Press ANY key to reboot...'; reset;" \
				"else echo Reading bootloader failed;" \
					"pause 'Press ANY key to reboot...'; reset; fi;" \
			"fi;" \
		"else echo SPI flash backup FAILED! Aborting ...;" \
			"pause 'Press ANY key to reboot...'; reset; fi\0"

#define IDEAPAD_BOOTMENU \
	IDEAPAD_FLASH_UBOOT \
	"bootmenu_0=mount internal storage=usb start && ums 0 mmc 0; bootmenu\0" \
	"bootmenu_1=mount external storage=usb start && ums 0 mmc 1; bootmenu\0" \
	"bootmenu_2=fastboot=echo Starting Fastboot protocol ...; fastboot usb 0; bootmenu\0" \
	"bootmenu_3=update bootloader=run flash_uboot\0" \
	"bootmenu_4=reboot RCM=enterrcm\0" \
	"bootmenu_5=reboot=reset\0" \
	"bootmenu_6=power off=poweroff\0" \
	"bootmenu_delay=-1\0"

#define BOARD_EXTRA_ENV_SETTINGS \
	"spi_size=0x400000\0" \
	"boot_block_size_r=0x200000\0" \
	"boot_block_size=0x1000\0" \
	"bootloader_file=u-boot-dtb-tegra.bin\0" \
	"button_cmd_0_name=Volume Down\0" \
	"button_cmd_0=bootmenu\0" \
	"button_cmd_1_name=Lid sensor\0" \
	"button_cmd_1=poweroff\0" \
	"partitions=name=emmc,start=0,size=-,uuid=${uuid_gpt_rootfs}\0" \
	IDEAPAD_BOOTMENU

/* Board-specific serial config */
#define CFG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
