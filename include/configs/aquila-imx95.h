/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright (c) Toradex */

#ifndef __AQUILA_IMX95_H
#define __AQUILA_IMX95_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

/* module has 8GB, 2GB from 0x80000000..0xffffffff, 6GB above.
 * Actual size is determined at runtime.
 */
#define SZ_6G	_AC(0x180000000, ULL)

/* The first 256MB of SDRAM is reserved for firmware (Cortex M7) */
#define PHYS_SDRAM_FW_RSVD	SZ_256M
#define CFG_SYS_INIT_RAM_ADDR	PHYS_SDRAM
#define CFG_SYS_INIT_RAM_SIZE	SZ_2M

#define CFG_SYS_SDRAM_BASE	PHYS_SDRAM
#define PHYS_SDRAM		(0x80000000 + PHYS_SDRAM_FW_RSVD)
#define PHYS_SDRAM_SIZE		(SZ_2G - PHYS_SDRAM_FW_RSVD)
#define PHYS_SDRAM_2_SIZE	SZ_6G

#define CFG_SYS_SECURE_SDRAM_BASE	0x8A000000 /* Secure DDR region for A55, SPL could use first 2MB */
#define CFG_SYS_SECURE_SDRAM_SIZE	0x06000000

#endif
