/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the SAMA5D3 Xplained board.
 *
 * Copyright (C) 2014 Atmel Corporation
 *		      Bo Shen <voice.shen@atmel.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>
#include "at91-sama5_common.h"

/*
 * This needs to be defined for the OHCI code to work but it is defined as
 * ATMEL_ID_UHPHS in the CPU specific header files.
 */
#define ATMEL_ID_UHP			32

/*
 * Specify the clock enable bit in the PMC_SCER register.
 */
#define ATMEL_PMC_UHP			(1 <<  6)

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE           0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x10000000

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_INIT_SP_ADDR		0x318000
#else
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 - GENERATED_GBL_DATA_SIZE)
#endif

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x60000000
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#endif

/* USB */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_UPLL
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00600000
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"SAMA5D3 Xplained"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#endif

/* SPL */
#define CONFIG_SPL_MAX_SIZE		0x18000
#define CONFIG_SPL_BSS_START_ADDR	0x20000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000
#define CONFIG_SYS_SPL_MALLOC_START	0x20080000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000

/* size of u-boot.bin to load */
#define CONFIG_SYS_MONITOR_LEN		(2 * SZ_512K)

#ifdef CONFIG_SD_BOOT
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"u-boot.img"
#endif

/* Falcon boot support on raw MMC */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0x100  /* 128 KiB */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	(CONFIG_CMD_SPL_WRITE_SIZE / 512)
/* U-Boot proper stored by default at 0x200 (256 KiB) */
#define CONFIG_SYS_SPL_ARGS_ADDR		0x22000000

/* Falcon boot support on FAT on MMC */
#define CONFIG_SPL_FS_LOAD_ARGS_NAME		"args"
#define CONFIG_SPL_FS_LOAD_KERNEL_NAME		"uImage"

/* Falcon boot support on raw NAND */
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS		0x1a0000

#endif
