/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright (c) Toradex */

#ifndef __VERDIN_IMX95_H
#define __VERDIN_IMX95_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

/* For 32GB modules: 2GB from 0x80000000..0xffffffff, 30GB above.
 * Actual size is determined at runtime.
 */
#define SZ_30G	_AC(0x780000000, ULL)

/* The first 256MB of SDRAM is reserved for firmware (Cortex M7) */
#define PHYS_SDRAM_FW_RSVD	SZ_256M
#define CFG_SYS_INIT_RAM_ADDR	PHYS_SDRAM
#define CFG_SYS_INIT_RAM_SIZE	SZ_2M

#define CFG_SYS_SDRAM_BASE	PHYS_SDRAM
#define PHYS_SDRAM		(0x80000000 + PHYS_SDRAM_FW_RSVD)
#define PHYS_SDRAM_SIZE		(SZ_2G - PHYS_SDRAM_FW_RSVD)
#define PHYS_SDRAM_2_SIZE	SZ_30G

#define WDOG_BASE_ADDR		WDG3_BASE_ADDR

#endif
