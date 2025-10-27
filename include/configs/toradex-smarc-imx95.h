/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright (C) 2025 Toradex */

#ifndef __IMX95_TORADEX_SMARC_H
#define __IMX95_TORADEX_SMARC_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

/* module has 8GB, 2GB from 0x80000000..0xffffffff, 6GB above */
#define SZ_6G	_AC(0x180000000, ULL)

/* first 256MB reserved for firmware */
#define CFG_SYS_INIT_RAM_ADDR	0x90000000
#define CFG_SYS_INIT_RAM_SIZE	SZ_2M

#define CFG_SYS_SDRAM_BASE	0x90000000
#define PHYS_SDRAM		0x90000000
#define PHYS_SDRAM_SIZE		(SZ_2G - SZ_256M)
#define PHYS_SDRAM_2_SIZE	SZ_6G

#define WDOG_BASE_ADDR	WDG3_BASE_ADDR

#endif
