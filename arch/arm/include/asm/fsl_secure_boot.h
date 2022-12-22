/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#ifndef __FSL_SECURE_BOOT_H
#define __FSL_SECURE_BOOT_H

#ifdef CONFIG_CHAIN_OF_TRUST
#ifndef CONFIG_SPL_BUILD
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
