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
#else /* !CONFIG_SPL_BUILD  */
#define CONFIG_SYS_NS16550_REG_SIZE     (-1)
#endif /* CONFIG_SPL_BUILD */

/* NAND */
#if defined(CONFIG_NAND)
#define CONFIG_NAND_OMAP_GPMC
#define CONFIG_SYS_FLASH_BASE		NAND_BASE
#define CONFIG_SYS_MAX_NAND_DEVICE      1
#define CONFIG_SYS_NAND_BUSWIDTH_16BIT
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
#define SMNAND_ENV_OFFSET               0x260000    /* environment starts here */
#define CONFIG_SYS_ENV_SECT_SIZE        (128 << 10) /* 128 KiB */
#define CONFIG_ENV_OFFSET               SMNAND_ENV_OFFSET
#define CONFIG_ENV_ADDR                 SMNAND_ENV_OFFSET
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
#define CONFIG_USB_ETHER

/* USB EHCI */
#define CONFIG_SYS_USB_FAT_BOOT_PARTITION  1

/* SMSC911x Ethernet */
#if defined(CONFIG_CMD_NET)
#define CONFIG_SMC911X
#define CONFIG_SMC911X_32_BIT
#define CONFIG_SMC911X_BASE             0x2C000000
#endif /* CONFIG_CMD_NET */

/* Environment */
#define CONFIG_PREBOOT                  "usb start"

#define CONFIG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
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
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loaduimage=setenv bootfile uImage; " \
		"fatload mmc ${mmcdev} ${loadaddr} uImage\0" \
	"loadzimage=setenv bootfile zImage; " \
		"fatload mmc ${mmcdev} ${loadaddr} zImage\0" \
	"loaddtb=fatload mmc ${mmcdev} ${fdtaddr} " CONFIG_DEFAULT_FDT_FILE "\0" \
	"mmcboot=echo Booting ${bootfile} from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr} - ${fdtaddr}\0" \
	"mmcbootz=echo Booting ${bootfile} from mmc ...; " \
		"run mmcargs; " \
		"bootz ${loadaddr} - ${fdtaddr}\0" \
	"nandboot=echo Booting uImage from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} kernel; " \
		"nand read ${fdtaddr} dtb; " \
		"bootm ${loadaddr} - ${fdtaddr}\0"

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"if run loadbootenv; then " \
			"run importbootenv; " \
			"if test -n $uenvcmd; then " \
				"echo Running uenvcmd ...; " \
				"run uenvcmd; " \
			"fi; " \
		"else " \
			"if run loadzimage && run loaddtb; then " \
				"run mmcbootz; fi; " \
			"if run loaduimage && run loaddtb; then " \
				"run mmcboot; fi; " \
			"run nandboot; " \
		"fi; " \
	"else run nandboot; fi"

#endif /* __CONFIG_H */
