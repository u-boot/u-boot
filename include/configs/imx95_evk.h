/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2025 NXP
 */

#ifndef __IMX95_EVK_H
#define __IMX95_EVK_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_INIT_RAM_ADDR	0x90000000
#define CFG_SYS_INIT_RAM_SIZE	0x200000

#define CFG_SYS_SDRAM_BASE		0x90000000
#define PHYS_SDRAM				0x90000000
/* Totally 16GB */
#define PHYS_SDRAM_SIZE			0x70000000 /* 2GB - 256MB DDR */
#define PHYS_SDRAM_2_SIZE		0x380000000 /* 14GB */

#define WDOG_BASE_ADDR			WDG3_BASE_ADDR

#endif
