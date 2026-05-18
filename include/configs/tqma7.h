/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 *
 * Copyright (c) 2016-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Steffen Doster
 *
 * Configuration settings for the TQ-Systems TQMa7x SOM
 */

#ifndef __TQMA7_CONFIG_H
#define __TQMA7_CONFIG_H

#include "mx7_common.h"
#include <linux/build_bug.h>

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR	0

/*
 * 128 MiB offset as recommended in Linux' `Documentation/arch/arm/booting.rst`
 * TQMA7_FDT_ADDRESS = MMDC0_ARB_BASE_ADDR + 0x8000000
 */
#define TQMA7_FDT_ADDRESS		0x88000000
/* FDT_OVERLAY_ADDR = (TQMA7_FDT_ADDRESS + SZ_256K) */
#define FDT_OVERLAY_ADDR		0x88040000
/*
 * DTB is loaded at 128 MiB, so use just 16 MiB more
 * TQMA7_INITRD_ADDRESS = (TQMA7_FDT_ADDRESS + SZ_16M)
 */
#define TQMA7_INITRD_ADDRESS		0x89000000

#ifndef __ASSEMBLY__

static_assert(TQMA7_FDT_ADDRESS == (MMDC0_ARB_BASE_ADDR + 0x8000000));
static_assert(FDT_OVERLAY_ADDR == (TQMA7_FDT_ADDRESS + SZ_256K));
static_assert(TQMA7_INITRD_ADDRESS == (TQMA7_FDT_ADDRESS + SZ_16M));

#endif

#define TQMA7_UBOOT_OFFSET		SZ_1K
#define TQMA7_MMC_UBOOT_SECTOR_START	0x2
#define TQMA7_MMC_UBOOT_SECTOR_COUNT	0x7fe
#define TQMA7_SPI_FLASH_SECTOR_SIZE	SZ_64K
#define TQMA7_SPI_UBOOT_START		0x1000
#define TQMA7_SPI_UBOOT_SIZE		0xf0000

/* Physical Memory Map */
#define PHYS_SDRAM		MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE	PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* u-boot.img base address for SPI-NOR boot */
#define CFG_SYS_UBOOT_BASE	(QSPI0_ARB_BASE_ADDR + TQMA7_UBOOT_OFFSET + CONFIG_SPL_PAD_TO)

/*
 * All the defines above are for the TQMa7x SoM
 *
 * Now include the baseboard specific configuration
 */

#if IS_ENABLED(CONFIG_MBA7)
#include "tqma7_mba7.h"
#else
#error "No baseboard for the TQMa7x SOM defined!"
#endif

#endif /* __TQMA7_CONFIG_H */
