/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
