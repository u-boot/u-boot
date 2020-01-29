/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2012,2015 Stephen Warren
 */

#ifndef _BCM2835_SDHCI_H_
#define _BCM2835_SDHCI_H_

#include <asm/arch/base.h>

#define BCM2835_SDHCI_PHYSADDR ({ BUG_ON(!rpi_bcm283x_base); \
				  rpi_bcm283x_base + 0x00300000; })

int bcm2835_sdhci_init(u32 regbase, u32 emmc_freq);

#endif
