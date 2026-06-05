/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2016-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Marco Felsch, Nora Schiffer, Max Merchel
 *
 * Configuration settings for the TQ-Systems TQMa6UL[L]x[L] SOM family.
 */

#ifndef __TQMA6UL_CONFIG_H
#define __TQMA6UL_CONFIG_H

#include <linux/build_bug.h>

#include "mx6_common.h"

#define TQMA6UL_MMC_UBOOT_SECTOR_START	0x2
#define TQMA6UL_MMC_UBOOT_SECTOR_COUNT	0x7fe

#define TQMA6UL_SPI_FLASH_SECTOR_SIZE	SZ_64K
#define TQMA6UL_SPI_UBOOT_START		SZ_4K
#define TQMA6UL_SPI_UBOOT_SIZE		0xf0000

/* 128 MiB offset as suggested in ARM related Linux docs */
#define TQMA6UL_FDT_ADDRESS		0x88000000

/* 256KiB above TQMA6UL_FDT_ADDRESS (TQMA6UL_FDT_ADDRESS + SZ_256K) */
#define TQMA6UL_FDT_OVERLAY_ADDR	0x88040000

/* 16MiB above TQMA6UL_FDT_ADDRESS (TQMA6UL_FDT_ADDRESS + SZ_16M) */
#define TQMA6UL_INITRD_ADDRESS		0x89000000

#ifndef __ASSEMBLY__
static_assert(TQMA6UL_FDT_OVERLAY_ADDR == (TQMA6UL_FDT_ADDRESS + SZ_256K));
static_assert(TQMA6UL_INITRD_ADDRESS == (TQMA6UL_FDT_ADDRESS + SZ_16M));
#endif

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR		IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE		IRAM_SIZE

/* u-boot.img base address for SPI-NOR boot */
#define CFG_SYS_UBOOT_BASE	(QSPI0_AMBA_BASE + SZ_4K + CONFIG_SPL_PAD_TO)

#define CFG_SYS_INIT_SP_OFFSET \
	(CFG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CFG_SYS_INIT_SP_ADDR \
	(CFG_SYS_INIT_RAM_ADDR + CFG_SYS_INIT_SP_OFFSET)

#endif /* __TQMA6UL_CONFIG_H */
