/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <asm/eth-raw-os.h>
#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <net.h>

DECLARE_GLOBAL_DATA_PTR;

static int reply_arp;
static struct in_addr arp_ip;

static int sb_eth_raw_start(struct udevice *dev)
{
	struct eth_sandbox_raw_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	const char *interface;

	debug("eth_sandbox_raw: Start\n");

	interface = fdt_getprop(gd->fdt_blob, dev->of_offset,
					    "host-raw-interface", NULL);
	if (interface == NULL)
		return -EINVAL;

	if (strcmp(interface, "lo") == 0) {
		priv->local = 1;
		setenv("ipaddr", "127.0.0.1");
		setenv("serverip", "127.0.0.1");
	}
	return sandbox_eth_raw_os_start(interface, pdata->enetaddr, priv);
}

static int sb_eth_raw_send(struct udevice *dev, void *packet, int length)
{
	struct eth_sandbox_raw_priv *priv = dev_get_priv(dev);

	debug("eth_sandbox_raw: Send packet %d\n", length);

	if (priv->local) {
		struct ethernet_hdr *eth = packet;

		if (ntohs(eth->et_protlen) == PROT_ARP) {
			struct arp_hdr *arp = packet + ETHER_HDR_SIZE;

			/**
			 * localhost works on a higher-level API in Linux than
			 * ARP packets, so fake it
			 */
			arp_ip = net_read_ip(&arp->ar_tpa);
			reply_arp = 1;
			return 0;
		}
		packet += ETHER_HDR_SIZE;
		length -= ETHER_HDR_SIZE;
	}
	return sandbox_eth_raw_os_send(packet, length, priv);
}

static int sb_eth_raw_recv(struct udevice *dev, uchar **packetp)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct eth_sandbox_raw_priv *priv = dev_get_priv(dev);
	int retval = 0;
	int length;

	if (reply_arp) {
		struct arp_hdr *arp = (void *)net_rx_packets[0] +
			ETHER_HDR_SIZE;

		/*
		 * Fake an ARP response. The u-boot network stack is sending an
		 * ARP request (to find the MAC address to address the actual
		 * packet to) and requires an ARP response to continue. Since
		 * this is the localhost interface, there is no Etherent level
		 * traffic at all, so there is no way to send an ARP request or
		 * to get a response. For this reason we fake the response to
		 * make the u-boot network stack happy.
		 */
		arp->ar_hrd = htons(ARP_ETHER);
		arp->ar_pro = htons(PROT_IP);
		arp->ar_hln = ARP_HLEN;
		arp->ar_pln = ARP_PLEN;
		arp->ar_op = htons(ARPOP_REPLY);
		/* Any non-zero MAC address will work */
		memset(&arp->ar_sha, 0x01, ARP_HLEN);
		/* Use whatever IP we were looking for (always 127.0.0.1?) */
		net_write_ip(&arp->ar_spa, arp_ip);
		memcpy(&arp->ar_tha, pdata->enetaddr, ARP_HLEN);
		net_write_ip(&arp->ar_tpa, net_ip);
		length = ARP_HDR_SIZE;
	} else {
		/* If local, the Ethernet header won't be included; skip it */
		uchar *pktptr = priv->local ?
			net_rx_packets[0] + ETHER_HDR_SIZE : net_rx_packets[0];

		retval = sandbox_eth_raw_os_recv(pktptr, &length, priv);
	}

	if (!retval && length) {
		if (priv->local) {
			struct ethernet_hdr *eth = (void *)net_rx_packets[0];

			/* Fill in enough of the missing Ethernet header */
			memcpy(eth->et_dest, pdata->enetaddr, ARP_HLEN);
			memset(eth->et_src, 0x01, ARP_HLEN);
			eth->et_protlen = htons(reply_arp ? PROT_ARP : PROT_IP);
			reply_arp = 0;
			length += ETHER_HDR_SIZE;
		}

		debug("eth_sandbox_raw: received packet %d\n",
		      length);
		*packetp = net_rx_packets[0];
		return length;
	}
	return retval;
}

static void sb_eth_raw_stop(struct udevice *dev)
{
	struct eth_sandbox_raw_priv *priv = dev_get_priv(dev);

	debug("eth_sandbox_raw: Stop\n");

	sandbox_eth_raw_os_stop(priv);
}

static const struct eth_ops sb_eth_raw_ops = {
	.start			= sb_eth_raw_start,
	.send			= sb_eth_raw_send,
	.recv			= sb_eth_raw_recv,
	.stop			= sb_eth_raw_stop,
};

static int sb_eth_raw_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);

	pdata->iobase = dev_get_addr(dev);
	return 0;
}

static const struct udevice_id sb_eth_raw_ids[] = {
	{ .compatible = "sandbox,eth-raw" },
	{ }
};

U_BOOT_DRIVER(eth_sandbox_raw) = {
	.name	= "eth_sandbox_raw",
	.id	= UCLASS_ETH,
	.of_match = sb_eth_raw_ids,
	.ofdata_to_platdata = sb_eth_raw_ofdata_to_platdata,
	.ops	= &sb_eth_raw_ops,
	.priv_auto_alloc_size = sizeof(struct eth_sandbox_raw_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
