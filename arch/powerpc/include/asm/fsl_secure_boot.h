/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FSL_SECURE_BOOT_H
#define __FSL_SECURE_BOOT_H
#include <asm/config_mpc85xx.h>

#ifdef CONFIG_SECURE_BOOT

#ifndef CONFIG_FIT_SIGNATURE
#define CONFIG_CHAIN_OF_TRUST
#endif

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
	defined(CONFIG_T104xD4QDS) || \
	defined(CONFIG_T104xRDB) || \
	defined(CONFIG_T104xD4RDB) || \
	defined(CONFIG_PPC_T1023) || \
	defined(CONFIG_PPC_T1024)
#define CONFIG_SYS_CPC_REINIT_F
#define CONFIG_KEY_REVOCATION
#undef CONFIG_SYS_INIT_L3_ADDR
#define CONFIG_SYS_INIT_L3_ADDR			0xbff00000
#endif

#if defined(CONFIG_RAMBOOT_PBL)
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

#if defined(CONFIG_FSL_CORENET) && !defined(CONFIG_SYS_RAMBOOT)
/* The key used for verification of next level images
 * is picked up from an Extension Table which has
 * been verified by the ISBC (Internal Secure boot Code)
 * in boot ROM of the SoC.
 * The feature is only applicable in case of NOR boot and is
 * not applicable in case of RAMBOOT (NAND, SD, SPI).
 */
#define CONFIG_FSL_ISBC_KEY_EXT
#endif
#endif /* #ifdef CONFIG_SECURE_BOOT */

#ifdef CONFIG_CHAIN_OF_TRUST

#define CONFIG_CMD_ESBC_VALIDATE
#define CONFIG_CMD_BLOB
#define CONFIG_FSL_SEC_MON
#define CONFIG_SHA_PROG_HW_ACCEL
#define CONFIG_RSA_FREESCALE_EXP

#ifndef CONFIG_FSL_CAAM
#define CONFIG_FSL_CAAM
#endif

/* fsl_setenv_chain_of_trust() must be called from
 * board_late_init()
 */
#ifndef CONFIG_BOARD_LATE_INIT
#define CONFIG_BOARD_LATE_INIT
#endif

/* If Boot Script is not on NOR and is required to be copied on RAM */
#ifdef CONFIG_BOOTSCRIPT_COPY_RAM
#define CONFIG_BS_HDR_ADDR_RAM		0x00010000
#define CONFIG_BS_HDR_ADDR_FLASH	0x00800000
#define CONFIG_BS_HDR_SIZE		0x00002000
#define CONFIG_BS_ADDR_RAM		0x00012000
#define CONFIG_BS_ADDR_FLASH		0x00802000
#define CONFIG_BS_SIZE			0x00001000

#define CONFIG_BOOTSCRIPT_HDR_ADDR	CONFIG_BS_HDR_ADDR_RAM
#else

/* The bootscript header address is different for B4860 because the NOR
 * mapping is different on B4 due to reduced NOR size.
 */
#if defined(CONFIG_B4860QDS)
#define CONFIG_BOOTSCRIPT_HDR_ADDR	0xecc00000
#elif defined(CONFIG_FSL_CORENET)
#define CONFIG_BOOTSCRIPT_HDR_ADDR	0xe8e00000
#elif defined(CONFIG_BSC9132QDS)
#define CONFIG_BOOTSCRIPT_HDR_ADDR	0x88020000
#elif defined(CONFIG_C29XPCIE)
#define CONFIG_BOOTSCRIPT_HDR_ADDR	0xec020000
#else
#define CONFIG_BOOTSCRIPT_HDR_ADDR	0xee020000
#endif

#endif /* #ifdef CONFIG_BOOTSCRIPT_COPY_RAM */

#include <config_fsl_chain_trust.h>
#endif /* #ifdef CONFIG_CHAIN_OF_TRUST */
#endif
