/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FSL_SECURE_BOOT_H
#define __FSL_SECURE_BOOT_H

#ifdef CONFIG_SECURE_BOOT
#if defined(CONFIG_FSL_CORENET)
#define CONFIG_SYS_PBI_FLASH_BASE		0xc0000000
#elif defined(CONFIG_BSC9132QDS)
#define CONFIG_SYS_PBI_FLASH_BASE		0xc8000000
#else
#define CONFIG_SYS_PBI_FLASH_BASE		0xce000000
#endif
#define CONFIG_SYS_PBI_FLASH_WINDOW		0xcff80000

#if defined(CONFIG_B4860QDS) || \
	defined(CONFIG_T4240QDS) || \
	defined(CONFIG_T2080QDS) || \
	defined(CONFIG_T2080RDB) || \
	defined(CONFIG_T1040QDS) || \
	defined(CONFIG_T104xRDB)
#define CONFIG_SYS_CPC_REINIT_F
#undef CONFIG_SYS_INIT_L3_ADDR
#define CONFIG_SYS_INIT_L3_ADDR			0xbff00000
#endif

#endif
#endif
