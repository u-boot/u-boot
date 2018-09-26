/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 */

#ifndef __ETH_H
#define __ETH_H

void sandbox_eth_disable_response(int index, bool disable);

void sandbox_eth_skip_timeout(void);

/*
 * sandbox_eth_arp_req_to_reply()
 *
 * Check for an arp request to be sent. If so, inject a reply
 *
 * @dev: device that received the packet
 * @packet: pointer to the received pacaket buffer
 * @len: length of received packet
 * @return 0 if injected, -EAGAIN if not
 */
int sandbox_eth_arp_req_to_reply(struct udevice *dev, void *packet,
				 unsigned int len);

/*
 * sandbox_eth_ping_req_to_reply()
 *
 * Check for a ping request to be sent. If so, inject a reply
 *
 * @dev: device that received the packet
 * @packet: pointer to the received pacaket buffer
 * @len: length of received packet
 * @return 0 if injected, -EAGAIN if not
 */
int sandbox_eth_ping_req_to_reply(struct udevice *dev, void *packet,
				  unsigned int len);

#endif /* __ETH_H */
