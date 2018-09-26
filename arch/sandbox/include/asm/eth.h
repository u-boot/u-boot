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

/**
 * A packet handler
 *
 * dev - device pointer
 * pkt - pointer to the "sent" packet
 * len - packet length
 */
typedef int sandbox_eth_tx_hand_f(struct udevice *dev, void *pkt,
				   unsigned int len);

/*
 * Set packet handler
 *
 * handler - The func ptr to call on send. If NULL, set to default handler
 */
void sandbox_eth_set_tx_handler(int index, sandbox_eth_tx_hand_f *handler);

#endif /* __ETH_H */
