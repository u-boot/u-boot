/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#ifndef __FSL_SECURE_BOOT_H
#define __FSL_SECURE_BOOT_H

#ifdef CONFIG_CHAIN_OF_TRUST
#ifndef CONFIG_SPL_BUILD
#ifndef CONFIG_SYS_RAMBOOT
/* The key used for verification of next level images
 * is picked up from an Extension Table which has
 * been verified by the ISBC (Internal Secure boot Code)
 * in boot ROM of the SoC.
 * The feature is only applicable in case of NOR boot and is
 * not applicable in case of RAMBOOT (NAND, SD, SPI).
 * For LS, this feature is available for all device if IE Table
 * is copied to XIP memory
 * Also, for LS, ISBC doesn't verify this table.
 */
#define CONFIG_FSL_ISBC_KEY_EXT

#endif

#ifdef CONFIG_FSL_LS_PPA
/* Define the key hash here if SRK used for signing PPA image is
 * different from SRK hash put in SFP used for U-Boot.
 * Example
 * #define PPA_KEY_HASH \
 *	"41066b564c6ffcef40ccbc1e0a5d0d519604000c785d97bbefd25e4d288d1c8b"
 */
#define PPA_KEY_HASH		NULL
#endif /* ifdef CONFIG_FSL_LS_PPA */

#endif /* #ifndef CONFIG_SPL_BUILD */
#endif /* #ifdef CONFIG_CHAIN_OF_TRUST */
#endif
