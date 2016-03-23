/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FSL_SECURE_BOOT_H
#define __FSL_SECURE_BOOT_H

#ifdef CONFIG_SECURE_BOOT

#ifndef CONFIG_FIT_SIGNATURE
#define CONFIG_CHAIN_OF_TRUST
#endif

#endif

#ifdef CONFIG_CHAIN_OF_TRUST
#define CONFIG_CMD_ESBC_VALIDATE
#define CONFIG_CMD_BLOB
#define CONFIG_CMD_HASH
#define CONFIG_FSL_SEC_MON
#define CONFIG_SHA_HW_ACCEL
#define CONFIG_SHA_PROG_HW_ACCEL
#define CONFIG_RSA_FREESCALE_EXP

#ifndef CONFIG_FSL_CAAM
#define CONFIG_FSL_CAAM
#endif

#define CONFIG_KEY_REVOCATION
#ifndef CONFIG_SYS_RAMBOOT
/* The key used for verification of next level images
 * is picked up from an Extension Table which has
 * been verified by the ISBC (Internal Secure boot Code)
 * in boot ROM of the SoC.
 * The feature is only applicable in case of NOR boot and is
 * not applicable in case of RAMBOOT (NAND, SD, SPI).
 */
#ifndef CONFIG_ESBC_HDR_LS
/* Current Key EXT feature not available in LS ESBC Header */
#define CONFIG_FSL_ISBC_KEY_EXT
#endif

#endif

#if defined(CONFIG_LS1043A) || defined(CONFIG_LS2080A) ||\
	defined(CONFIG_LS2085A)
/* For LS1043 (ARMv8), ESBC image Address in Header is 64 bit
 * Similiarly for LS2080 and LS2085
 */
#define CONFIG_ESBC_ADDR_64BIT
#endif

#define CONFIG_EXTRA_ENV \
	"setenv fdt_high 0xcfffffff;"	\
	"setenv initrd_high 0xcfffffff;"	\
	"setenv hwconfig \'fsl_ddr:ctlr_intlv=null,bank_intlv=null\';"

/* The address needs to be modified according to NOR memory map */
#define CONFIG_BOOTSCRIPT_HDR_ADDR	0x600a0000

#include <config_fsl_chain_trust.h>
#endif /* #ifdef CONFIG_CHAIN_OF_TRUST */
#endif
