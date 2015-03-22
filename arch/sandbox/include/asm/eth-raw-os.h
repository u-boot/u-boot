/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef __ETH_RAW_OS_H
#define __ETH_RAW_OS_H

/**
 * struct eth_sandbox_raw_priv - raw socket session
 *
 * sd: socket descriptor - the open socket during a session
 * device: struct sockaddr_ll - the host interface packets move to/from
 */
struct eth_sandbox_raw_priv {
	int sd;
	void *device;
};

int sandbox_eth_raw_os_start(const char *ifname, unsigned char *ethmac,
			    struct eth_sandbox_raw_priv *priv);
int sandbox_eth_raw_os_send(void *packet, int length,
			    const struct eth_sandbox_raw_priv *priv);
int sandbox_eth_raw_os_recv(void *packet, int *length,
			    const struct eth_sandbox_raw_priv *priv);
void sandbox_eth_raw_os_stop(struct eth_sandbox_raw_priv *priv);

#endif /* __ETH_RAW_OS_H */
