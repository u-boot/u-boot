/*
 * include/configs/porter.h
 *     This file is Porter board configuration.
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
 * Copyright (C) 2015 Cogent Embedded, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef __PORTER_H
#define __PORTER_H

#undef DEBUG
#define CONFIG_ARCH_RMOBILE_BOARD_STRING "Porter"

#include "rcar-gen2-common.h"

#define CONFIG_SYS_INIT_SP_ADDR		0x4f000000
#define STACK_AREA_SIZE			0x00100000
#define LOW_LEVEL_MERAM_STACK \
		(CONFIG_SYS_INIT_SP_ADDR + STACK_AREA_SIZE - 4)

/* MEMORY */
#define RCAR_GEN2_SDRAM_BASE		0x40000000
#define RCAR_GEN2_SDRAM_SIZE		(2048u * 1024 * 1024)
#define RCAR_GEN2_UBOOT_SDRAM_SIZE	(1024u * 1024 * 1024)

/* SCIF */

/* FLASH */
#define CONFIG_SPI
#define CONFIG_SPI_FLASH_QUAD

/* SH Ether */
#define CONFIG_SH_ETHER_USE_PORT	0
#define CONFIG_SH_ETHER_PHY_ADDR	0x1
#define CONFIG_SH_ETHER_PHY_MODE PHY_INTERFACE_MODE_RMII
#define CONFIG_SH_ETHER_CACHE_WRITEBACK
#define CONFIG_SH_ETHER_CACHE_INVALIDATE
#define CONFIG_SH_ETHER_ALIGNE_SIZE	64
#define CONFIG_BITBANGMII
#define CONFIG_BITBANGMII_MULTI

/* Board Clock */
#define RMOBILE_XTAL_CLK	20000000u
#define CONFIG_SYS_CLK_FREQ	RMOBILE_XTAL_CLK
#define CONFIG_SH_TMU_CLK_FREQ	(CONFIG_SYS_CLK_FREQ / 2)

#define CONFIG_SYS_TMU_CLK_DIV	4

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"fdt_high=0xffffffff\0"		\
	"initrd_high=0xffffffff\0"

/* SPL support */
#define CONFIG_SPL_TEXT_BASE		0xe6304000
#define CONFIG_SPL_STACK		0xe6340000
#define CONFIG_SPL_MAX_SIZE		0x40000
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x140000

#endif /* __PORTER_H */
