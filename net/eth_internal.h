/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2001-2015
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Joe Hershberger, National Instruments
 */

#ifndef __ETH_INTERNAL_H
#define __ETH_INTERNAL_H

/* Do init that is common to driver model and legacy networking */
void eth_common_init(void);

int eth_mac_skip(int index);
void eth_current_changed(void);
void eth_set_dev(struct udevice *dev);
void eth_set_current_to_next(void);

#endif
