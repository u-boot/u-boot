/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2024 NXP
 */

#ifndef __IMX91_QSB_H
#define __IMX91_QSB_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

/* Link Definitions */

#define CFG_SYS_INIT_RAM_ADDR        0x80000000
#define CFG_SYS_INIT_RAM_SIZE        0x200000

#define CFG_SYS_SDRAM_BASE           0x80000000
#define PHYS_SDRAM                      0x80000000
#define PHYS_SDRAM_SIZE			0x40000000 /* 1GB DDR */

/* Using WDOG for reset */
#define WDOG_BASE_ADDR          WDG3_BASE_ADDR

#endif
