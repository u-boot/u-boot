/*
 * (C) Copyright 2012,2015 Stephen Warren
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _BCM2835_MSG_H
#define _BCM2835_MSG_H

/**
 * bcm2835_power_on_module() - power on an SoC module
 *
 * @module: ID of module to power on (BCM2835_MBOX_POWER_DEVID_...)
 * @return 0 if OK, -EIO on error
 */
int bcm2835_power_on_module(u32 module);

/**
 * bcm2835_get_mmc_clock() - get the frequency of the MMC clock
 *
 * @return clock frequency, or -ve on error
 */
int bcm2835_get_mmc_clock(void);

/**
 * bcm2835_get_video_size() - get the current display size
 *
 * @widthp: Returns the width in pixels
 * @heightp: Returns the height in pixels
 * @return 0 if OK, -ve on error
 */
int bcm2835_get_video_size(int *widthp, int *heightp);

#endif
