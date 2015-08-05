/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef __ETH_H
#define __ETH_H

void sandbox_eth_disable_response(int index, bool disable);

void sandbox_eth_skip_timeout(void);

#endif /* __ETH_H */
