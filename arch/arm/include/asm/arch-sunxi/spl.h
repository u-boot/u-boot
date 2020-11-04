/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 */
#ifndef	_ASM_ARCH_SPL_H_
#define	_ASM_ARCH_SPL_H_

#include <sunxi_image.h>

#define SPL_ADDR		CONFIG_SUNXI_SRAM_ADDRESS

/* The low 8-bits of the 'boot_media' field in the SPL header */
#define SUNXI_BOOTED_FROM_MMC0	0
#define SUNXI_BOOTED_FROM_NAND	1
#define SUNXI_BOOTED_FROM_MMC2	2
#define SUNXI_BOOTED_FROM_SPI	3
#define SUNXI_BOOTED_FROM_MMC0_HIGH	0x10
#define SUNXI_BOOTED_FROM_MMC2_HIGH	0x12

#define is_boot0_magic(addr)	(memcmp((void *)(addr), BOOT0_MAGIC, 8) == 0)

uint32_t sunxi_get_boot_device(void);

#endif
