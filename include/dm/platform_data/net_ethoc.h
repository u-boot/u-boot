/*
 * Copyright (C) 2016 Cadence Design Systems Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _ETHOC_H
#define _ETHOC_H

#include <net.h>

#ifdef CONFIG_DM_ETH

struct ethoc_eth_pdata {
	struct eth_pdata eth_pdata;
};

#endif

#endif /* _ETHOC_H */
