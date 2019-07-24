/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2012,2015 Stephen Warren
 */

#ifndef _BCM2835_SDHCI_H_
#define _BCM2835_SDHCI_H_

#define BCM2835_SDHCI_BASE (CONFIG_BCM283x_BASE + 0x00300000)

int bcm2835_sdhci_init(u32 regbase, u32 emmc_freq);

#endif
