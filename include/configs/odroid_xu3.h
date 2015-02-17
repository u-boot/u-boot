/*
 * Copyright (C) 2013 Samsung Electronics
 * Hyungwon Hwang <human.hwang@samsung.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __CONFIG_ODROID_XU3_H
#define __CONFIG_ODROID_XU3_H

#include "exynos5420-common.h"

#define CONFIG_SYS_PROMPT		"ODROID-XU3 # "
#define CONFIG_IDENT_STRING		" for ODROID-XU3"

#define CONFIG_BOARD_COMMON

#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_TEXT_BASE		0x43E00000

/* select serial console configuration */
#define CONFIG_SERIAL2			/* use SERIAL 2 */

#define TZPC_BASE_OFFSET		0x10000

#define CONFIG_CMD_MMC

#define CONFIG_NR_DRAM_BANKS	8
#define SDRAM_BANK_SIZE		(256UL << 20UL)	/* 256 MB */
/* Reserve the last 22 MiB for the secure firmware */
#define CONFIG_SYS_MEM_TOP_HIDE		(22UL << 20UL)
#define CONFIG_TZSW_RESERVED_DRAM_SIZE	CONFIG_SYS_MEM_TOP_HIDE

#define CONFIG_ENV_IS_IN_MMC

#undef CONFIG_ENV_SIZE
#undef CONFIG_ENV_OFFSET
#define CONFIG_ENV_SIZE			4096
#define CONFIG_ENV_OFFSET		(SZ_1K * 1280) /* 1.25 MiB offset */

#define CONFIG_SYS_INIT_SP_ADDR        (CONFIG_SYS_LOAD_ADDR - 0x1000000)

#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC2,115200n8\0"

/* USB */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_EXYNOS

/* FIXME: MUST BE REMOVED AFTER TMU IS TURNED ON */
#undef CONFIG_EXYNOS_TMU
#undef CONFIG_TMU_CMD_DTT

#endif	/* __CONFIG_H */
