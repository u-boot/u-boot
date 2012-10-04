/*
 * RMOBILE EtherMAC initialization.
 *
 * Copyright (C) 2012  Renesas Solutions Corp.
 * Copyright (C) 2012  Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */

#include <common.h>
#include <asm/errno.h>
#include <netdev.h>

int cpu_eth_init(bd_t *bis)
{
	int ret = -ENODEV;
#ifdef CONFIG_SH_ETHER
	ret = sh_eth_initialize(bis);
#endif
	return ret;
}
