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

#endif
