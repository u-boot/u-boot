/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 */

#ifndef __ETH_RAW_OS_H
#define __ETH_RAW_OS_H

/**
 * struct eth_sandbox_raw_priv - raw socket session
 *
 * sd: socket descriptor - the open socket during a session
 * device: struct sockaddr_ll - the host interface packets move to/from
 * local: 1 or 0 to select the local interface ('lo') or not
 * local_bindsd: socket descriptor to prevent the kernel from sending
 *		 a message to the server claiming the port is
 *		 unreachable
 * local_bind_udp_port: The UDP port number that we bound to
 */
struct eth_sandbox_raw_priv {
	int sd;
	void *device;
	int local;
	int local_bind_sd;
	unsigned short local_bind_udp_port;
};

int sandbox_eth_raw_os_start(const char *ifname, unsigned char *ethmac,
			    struct eth_sandbox_raw_priv *priv);
int sandbox_eth_raw_os_send(void *packet, int length,
			    struct eth_sandbox_raw_priv *priv);
int sandbox_eth_raw_os_recv(void *packet, int *length,
			    const struct eth_sandbox_raw_priv *priv);
void sandbox_eth_raw_os_stop(struct eth_sandbox_raw_priv *priv);

#endif /* __ETH_RAW_OS_H */
