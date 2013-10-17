/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FSL_SECURE_BOOT_H
#define __FSL_SECURE_BOOT_H

#if defined(CONFIG_FSL_CORENET)
#define CONFIG_SYS_PBI_FLASH_BASE		0xc0000000
#else
#define CONFIG_SYS_PBI_FLASH_BASE		0xce000000
#endif
#define CONFIG_SYS_PBI_FLASH_WINDOW		0xcff80000

#endif
