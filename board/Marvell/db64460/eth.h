/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * eth.h - header file for the polled mode GT ethernet driver
 */

#ifndef __EVB64460_ETH_H__
#define __EVB64460_ETH_H__

#include <asm/types.h>
#include <asm/io.h>
#include <asm/byteorder.h>
#include <common.h>

int db64460_eth0_poll(void);
int db64460_eth0_transmit(unsigned int s, volatile char *p);
void db64460_eth0_disable(void);
bool network_start(bd_t *bis);

#endif /* __EVB64460_ETH_H__ */
