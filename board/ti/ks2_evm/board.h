/*
 * K2HK EVM : Board common header
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _KS2_BOARD
#define _KS2_BOARD

#include <asm/ti-common/keystone_net.h>

extern struct eth_priv_t eth_priv_cfg[];

int get_num_eth_ports(void);
void spl_init_keystone_plls(void);

#endif
