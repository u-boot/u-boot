/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FSL_SECURE_BOOT_H
#define __FSL_SECURE_BOOT_H
#include <asm/config_mpc85xx.h>

#ifdef CONFIG_SECURE_BOOT
#define CONFIG_CMD_ESBC_VALIDATE
#define CONFIG_FSL_SEC_MON
#define CONFIG_SHA_PROG_HW_ACCEL
#define CONFIG_DM
#define CONFIG_RSA
#define CONFIG_RSA_FREESCALE_EXP
#ifndef CONFIG_FSL_CAAM
#define CONFIG_FSL_CAAM
#endif
#endif

#ifdef CONFIG_SECURE_BOOT
#if defined(CONFIG_FSL_CORENET)
#define CONFIG_SYS_PBI_FLASH_BASE		0xc0000000
#elif defined(CONFIG_BSC9132QDS)
#define CONFIG_SYS_PBI_FLASH_BASE		0xc8000000
#elif defined(CONFIG_C29XPCIE)
#define CONFIG_SYS_PBI_FLASH_BASE		0xcc000000
#else
#define CONFIG_SYS_PBI_FLASH_BASE		0xce000000
#endif
#define CONFIG_SYS_PBI_FLASH_WINDOW		0xcff80000

#if defined(CONFIG_B4860QDS) || \
	defined(CONFIG_T4240QDS) || \
	defined(CONFIG_T2080QDS) || \
	defined(CONFIG_T2080RDB) || \
	defined(CONFIG_T1040QDS) || \
	defined(CONFIG_T104xRDB) || \
	defined(CONFIG_PPC_T1023) || \
	defined(CONFIG_PPC_T1024)
#define CONFIG_SYS_CPC_REINIT_F
#define CONFIG_KEY_REVOCATION
#undef CONFIG_SYS_INIT_L3_ADDR
#define CONFIG_SYS_INIT_L3_ADDR			0xbff00000
#endif

#if defined(CONFIG_C29XPCIE)
#define CONFIG_KEY_REVOCATION
#endif

#if defined(CONFIG_PPC_P3041)	||	\
	defined(CONFIG_PPC_P4080) ||	\
	defined(CONFIG_PPC_P5020) ||	\
	defined(CONFIG_PPC_P5040) ||	\
	defined(CONFIG_PPC_P2041)
	#define	CONFIG_FSL_TRUST_ARCH_v1
#endif

#if defined(CONFIG_FSL_CORENET)
/* The key used for verification of next level images
 * is picked up from an Extension Table which has
 * been verified by the ISBC (Internal Secure boot Code)
 * in boot ROM of the SoC
 */
#define CONFIG_FSL_ISBC_KEY_EXT
#endif

#endif
#endif
