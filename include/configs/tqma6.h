/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2013-2014, 2017 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 *
 * Configuration settings for the TQ-Systems TQMa6<Q,D,DL,S> module.
 */

#ifndef __TQMA6_CONFIG_H
#define __TQMA6_CONFIG_H

#include "mx6_common.h"

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR	0

/* 128 MiB offset as in ARM related docu for linux suggested */
#define TQMA6_FDT_ADDRESS		0x18000000

/* 256KiB above TQMA6_FDT_ADDRESS (TQMA6_FDT_ADDRESS + SZ_256K) */
#define TQMA6_FDT_OVERLAY_ADDR		0x18040000

/* 16MiB above TQMA6_FDT_ADDRESS (TQMA6_FDT_ADDRESS + SZ_16M) */
#define TQMA6_INITRD_ADDRESS		0x19000000

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define TQMA6_MMC_UBOOT_SECTOR_START	0x2
#define TQMA6_MMC_UBOOT_SECTOR_COUNT	0x7fe

#define TQMA6_SPI_FLASH_SECTOR_SIZE	SZ_64K
#define TQMA6_SPI_UBOOT_START		0x400
#define TQMA6_SPI_UBOOT_SIZE		0xc0000

#endif /* __TQMA6_CONFIG_H */
