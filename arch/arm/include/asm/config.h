/*
 * Copyright 2009 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_CONFIG_H_
#define _ASM_CONFIG_H_

#define CONFIG_LMB
#define CONFIG_SYS_BOOT_RAMDISK_HIGH

#ifdef CONFIG_ARM64
#define CONFIG_PHYS_64BIT
#define CONFIG_STATIC_RELA
#endif

#ifdef CONFIG_FSL_LSCH3
#include <asm/arch-fsl-lsch3/config.h>
#endif

#if defined(CONFIG_LS102XA) || \
	defined(CONFIG_CPU_PXA27X) || \
	defined(CONFIG_CPU_MONAHANS) || \
	defined(CONFIG_CPU_PXA25X)
#include <asm/arch/config.h>
#endif

#endif
