/*
 * (C) Copyright 2012 Stephen Warren
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _BCM2835_SDHCI_H_
#define _BCM2835_SDHCI_H_

#define BCM2835_SDHCI_BASE 0x20300000

int bcm2835_sdhci_init(u32 regbase, u32 emmc_freq);

#endif
