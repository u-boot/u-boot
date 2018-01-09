/*
 * Configuration settings for the TI OMAP3 EVM board.
 *
 * Copyright (C) 2006-2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Author :
 *	Manikandan Pillai <mani.pillai@ti.com>
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
 * Manikandan Pillai <mani.pillai@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_NR_DRAM_BANKS            2 /* CS1 may or may not be populated */

#include <configs/ti_omap3_common.h>

/*
 * We are only ever GP parts and will utilize all of the "downloaded image"
 * area in SRAM which starts at 0x40200000 and ends at 0x4020FFFF (64KB).
 */
#undef CONFIG_SPL_TEXT_BASE
#define CONFIG_SPL_TEXT_BASE            0x40200000

#define CONFIG_SPL_FRAMEWORK

#define CONFIG_MISC_INIT_R
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/* Override OMAP3 serial console configuration */
#undef CONFIG_CONS_INDEX
#define CONFIG_CONS_INDEX               1
#define CONFIG_SYS_NS16550_COM1         OMAP34XX_UART1
#if defined(CONFIG_SPL_BUILD)
#undef CONFIG_SYS_NS16550_REG_SIZE
#endif /* CONFIG_SPL_BUILD */

/* NAND */
#if defined(CONFIG_NAND)
#define CONFIG_SYS_FLASH_BASE		NAND_BASE
#define CONFIG_SYS_MAX_NAND_DEVICE      1
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT      64
#define CONFIG_SYS_NAND_PAGE_SIZE       2048
#define CONFIG_SYS_NAND_OOBSIZE         64
#define CONFIG_SYS_NAND_BLOCK_SIZE      (128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS   NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS          {2, 3, 4, 5, 6, 7, 8, 9,\
                                         10, 11, 12, 13}
#define CONFIG_SYS_NAND_ECCSIZE         512
#define CONFIG_SYS_NAND_ECCBYTES        3
#define CONFIG_NAND_OMAP_ECCSCHEME      OMAP_ECC_BCH8_CODE_HW_DETECTION_SW
#define CONFIG_SYS_NAND_U_BOOT_OFFS     0x80000
#define CONFIG_ENV_IS_IN_NAND           1
#define CONFIG_ENV_SIZE                 (128 << 10) /* 128 KiB */
#define CONFIG_SYS_ENV_SECT_SIZE        (128 << 10) /* 128 KiB */
#define CONFIG_ENV_OFFSET               0x260000
#define CONFIG_ENV_ADDR                 0x260000
#define CONFIG_ENV_OVERWRITE
#define CONFIG_MTD_PARTITIONS           /* required for UBI partition support */
/* NAND: SPL falcon mode configs */
#if defined(CONFIG_SPL_OS_BOOT)
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS 0x280000
#endif /* CONFIG_SPL_OS_BOOT */
#endif /* CONFIG_NAND */

/* MUSB */
#define CONFIG_USB_OMAP3
#define CONFIG_USB_MUSB_OMAP2PLUS
#define CONFIG_USB_MUSB_PIO_ONLY

/* USB EHCI */
#define CONFIG_SYS_USB_FAT_BOOT_PARTITION  1

/* Environment */
#define CONFIG_PREBOOT                  "usb start"

#include <config_distro_defaults.h>

#define MEM_LAYOUT_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV

#if defined(CONFIG_NAND)
#define BOOTENV_DEV_NAND(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=" \
	"run nandboot\0"

#define BOOTENV_DEV_NAME_NAND(devtypeu, devtypel, instance) \
	#devtypel #instance " "
#endif /* CONFIG_NAND */

#define BOOTENV_DEV_UIMAGE_MMC(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=" \
		"setenv mmcdev " #instance"; " \
		"run mmcboot\0"

#define BOOTENV_DEV_NAME_UIMAGE_MMC(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#define BOOTENV_DEV_ZIMAGE_MMC(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=" \
		"setenv mmcdev " #instance"; " \
		"run mmcbootz\0"

#define BOOTENV_DEV_NAME_ZIMAGE_MMC(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(ZIMAGE_MMC, zimage_mmc, 0) \
	func(UIMAGE_MMC, uimage_mmc, 0) \
	func(NAND, nand, 0)

#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"fdt_high=0xffffffff\0" \
	"bootenv=uEnv.txt\0" \
	"optargs=\0" \
	"mmcdev=0\0" \
	"console=ttyO0,115200n8\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"${mtdparts} " \
		"${optargs} " \
		"root=/dev/mmcblk0p2 rw " \
		"rootfstype=ext4 rootwait\0" \
	"nandargs=setenv bootargs console=${console} " \
		"${mtdparts} " \
		"${optargs} " \
		"root=ubi0:rootfs rw ubi.mtd=rootfs noinitrd " \
		"rootfstype=ubifs rootwait\0" \
	"loadbootenv=fatload mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t ${loadaddr} ${filesize}\0" \
	"mmcbootenv=" \
		"mmc dev ${mmcdev}; " \
		"if mmc rescan && run loadbootenv; then " \
			"run importbootenv; " \
			"if test -n $uenvcmd; then " \
				"echo Running uenvcmd ...; " \
				"run uenvcmd; " \
			"fi; " \
		"fi\0" \
	"loaduimage=setenv bootfile uImage; " \
		"fatload mmc ${mmcdev} ${loadaddr} uImage\0" \
	"loadzimage=setenv bootfile zImage; " \
		"fatload mmc ${mmcdev} ${loadaddr} zImage\0" \
	"loaddtb=fatload mmc ${mmcdev} ${fdtaddr} ${fdtfile}\0" \
	"mmcboot=run mmcbootenv; " \
		"if run loaduimage && run loaddtb; then " \
			"echo Booting ${bootfile} from mmc ...; " \
			"run mmcargs; " \
			"bootm ${loadaddr} - ${fdtaddr}; " \
		"fi\0" \
	"mmcbootz=run mmcbootenv; " \
		"if run loadzimage && run loaddtb; then " \
			"echo Booting ${bootfile} from mmc ...; " \
			"run mmcargs; " \
			"bootz ${loadaddr} - ${fdtaddr};" \
		"fi\0" \
	"nandboot=echo Booting uImage from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} kernel; " \
		"nand read ${fdtaddr} dtb; " \
		"bootm ${loadaddr} - ${fdtaddr}\0" \
	BOOTENV

#endif /* __CONFIG_H */
