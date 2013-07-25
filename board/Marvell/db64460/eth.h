/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

int mv6446x_eth_initialize(bd_t *);

#endif /* __EVB64460_ETH_H__ */
