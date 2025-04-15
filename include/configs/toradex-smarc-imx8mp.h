/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright (C) 2024 Toradex */

#ifndef __TORADEX_SMARC_IMX8MP_H
#define __TORADEX_SMARC_IMX8MP_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CFG_MALLOC_F_ADDR	0x184000
#endif /* CONFIG_SPL_BUILD */

#define CFG_SYS_INIT_RAM_ADDR	0x40000000
#define CFG_SYS_INIT_RAM_SIZE	SZ_512K

/* i.MX 8M Plus supports max. 8GB memory in two albeit consecutive banks */
#define CFG_SYS_SDRAM_BASE	0x40000000
#define PHYS_SDRAM		0x40000000
#define PHYS_SDRAM_SIZE		(SZ_2G + SZ_1G)
#define PHYS_SDRAM_2		0x100000000
#define PHYS_SDRAM_2_SIZE	(SZ_4G + SZ_1G)

#endif /* __TORADEX_SMARC_IMX8MP_H */
