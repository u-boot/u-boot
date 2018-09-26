// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <net.h>
#include <asm/test.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct eth_sandbox_priv - memory for sandbox mock driver
 *
 * fake_host_hwaddr: MAC address of mocked machine
 * fake_host_ipaddr: IP address of mocked machine
 * disabled: Will not respond
 * recv_packet_buffer: buffer of the packet returned as received
 * recv_packet_length: length of the packet returned as received
 */
struct eth_sandbox_priv {
	uchar fake_host_hwaddr[ARP_HLEN];
	struct in_addr fake_host_ipaddr;
	bool disabled;
	uchar *recv_packet_buffer;
	int recv_packet_length;
};

static bool skip_timeout;

/*
 * sandbox_eth_disable_response()
 *
 * index - The alias index (also DM seq number)
 * disable - If non-zero, ignore sent packets and don't send mock response
 */
void sandbox_eth_disable_response(int index, bool disable)
{
	struct udevice *dev;
	struct eth_sandbox_priv *priv;
	int ret;

	ret = uclass_get_device(UCLASS_ETH, index, &dev);
	if (ret)
		return;

	priv = dev_get_priv(dev);
	priv->disabled = disable;
}

/*
 * sandbox_eth_skip_timeout()
 *
 * When the first packet read is attempted, fast-forward time
 */
void sandbox_eth_skip_timeout(void)
{
	skip_timeout = true;
}

/*
 * sandbox_eth_arp_req_to_reply()
 *
 * Check for an arp request to be sent. If so, inject a reply
 *
 * returns 0 if injected, -EAGAIN if not
 */
int sandbox_eth_arp_req_to_reply(struct udevice *dev, void *packet,
				 unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth = packet;
	struct arp_hdr *arp;
	struct ethernet_hdr *eth_recv;
	struct arp_hdr *arp_recv;

	if (ntohs(eth->et_protlen) != PROT_ARP)
		return -EAGAIN;

	arp = packet + ETHER_HDR_SIZE;

	if (ntohs(arp->ar_op) != ARPOP_REQUEST)
		return -EAGAIN;

	/* store this as the assumed IP of the fake host */
	priv->fake_host_ipaddr = net_read_ip(&arp->ar_tpa);

	/* Formulate a fake response */
	eth_recv = (void *)priv->recv_packet_buffer;
	memcpy(eth_recv->et_dest, eth->et_src, ARP_HLEN);
	memcpy(eth_recv->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	eth_recv->et_protlen = htons(PROT_ARP);

	arp_recv = (void *)eth_recv + ETHER_HDR_SIZE;
	arp_recv->ar_hrd = htons(ARP_ETHER);
	arp_recv->ar_pro = htons(PROT_IP);
	arp_recv->ar_hln = ARP_HLEN;
	arp_recv->ar_pln = ARP_PLEN;
	arp_recv->ar_op = htons(ARPOP_REPLY);
	memcpy(&arp_recv->ar_sha, priv->fake_host_hwaddr, ARP_HLEN);
	net_write_ip(&arp_recv->ar_spa, priv->fake_host_ipaddr);
	memcpy(&arp_recv->ar_tha, &arp->ar_sha, ARP_HLEN);
	net_copy_ip(&arp_recv->ar_tpa, &arp->ar_spa);

	priv->recv_packet_length = ETHER_HDR_SIZE + ARP_HDR_SIZE;

	return 0;
}

/*
 * sandbox_eth_ping_req_to_reply()
 *
 * Check for a ping request to be sent. If so, inject a reply
 *
 * returns 0 if injected, -EAGAIN if not
 */
int sandbox_eth_ping_req_to_reply(struct udevice *dev, void *packet,
				  unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth = packet;
	struct ip_udp_hdr *ip;
	struct icmp_hdr *icmp;
	struct ethernet_hdr *eth_recv;
	struct ip_udp_hdr *ipr;
	struct icmp_hdr *icmpr;

	if (ntohs(eth->et_protlen) != PROT_IP)
		return -EAGAIN;

	ip = packet + ETHER_HDR_SIZE;

	if (ip->ip_p != IPPROTO_ICMP)
		return -EAGAIN;

	icmp = (struct icmp_hdr *)&ip->udp_src;

	if (icmp->type != ICMP_ECHO_REQUEST)
		return -EAGAIN;

	/* reply to the ping */
	eth_recv = (void *)priv->recv_packet_buffer;
	memcpy(eth_recv, packet, len);
	ipr = (void *)eth_recv + ETHER_HDR_SIZE;
	icmpr = (struct icmp_hdr *)&ipr->udp_src;
	memcpy(eth_recv->et_dest, eth->et_src, ARP_HLEN);
	memcpy(eth_recv->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	ipr->ip_sum = 0;
	ipr->ip_off = 0;
	net_copy_ip((void *)&ipr->ip_dst, &ip->ip_src);
	net_write_ip((void *)&ipr->ip_src, priv->fake_host_ipaddr);
	ipr->ip_sum = compute_ip_checksum(ipr, IP_HDR_SIZE);

	icmpr->type = ICMP_ECHO_REPLY;
	icmpr->checksum = 0;
	icmpr->checksum = compute_ip_checksum(icmpr, ICMP_HDR_SIZE);

	priv->recv_packet_length = len;

	return 0;
}

static int sb_eth_start(struct udevice *dev)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);

	debug("eth_sandbox: Start\n");

	priv->recv_packet_buffer = net_rx_packets[0];

	return 0;
}

static int sb_eth_send(struct udevice *dev, void *packet, int length)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);

	debug("eth_sandbox: Send packet %d\n", length);

	if (priv->disabled)
		return 0;

	if (!sandbox_eth_arp_req_to_reply(dev, packet, length))
		return 0;
	if (!sandbox_eth_ping_req_to_reply(dev, packet, length))
		return 0;
}

static int sb_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);

	if (skip_timeout) {
		sandbox_timer_add_offset(11000UL);
		skip_timeout = false;
	}

	if (priv->recv_packet_length) {
		int lcl_recv_packet_length = priv->recv_packet_length;

		debug("eth_sandbox: received packet %d\n",
		      priv->recv_packet_length);
		priv->recv_packet_length = 0;
		*packetp = priv->recv_packet_buffer;
		return lcl_recv_packet_length;
	}
	return 0;
}

static void sb_eth_stop(struct udevice *dev)
{
	debug("eth_sandbox: Stop\n");
}

static int sb_eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);

	debug("eth_sandbox %s: Write HW ADDR - %pM\n", dev->name,
	      pdata->enetaddr);
	return 0;
}

static const struct eth_ops sb_eth_ops = {
	.start			= sb_eth_start,
	.send			= sb_eth_send,
	.recv			= sb_eth_recv,
	.stop			= sb_eth_stop,
	.write_hwaddr		= sb_eth_write_hwaddr,
};

static int sb_eth_remove(struct udevice *dev)
{
	return 0;
}

static int sb_eth_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	const u8 *mac;

	pdata->iobase = dev_read_addr(dev);

	mac = dev_read_u8_array_ptr(dev, "fake-host-hwaddr", ARP_HLEN);
	if (!mac) {
		printf("'fake-host-hwaddr' is missing from the DT\n");
		return -EINVAL;
	}
	memcpy(priv->fake_host_hwaddr, mac, ARP_HLEN);
	priv->disabled = false;

	return 0;
}

static const struct udevice_id sb_eth_ids[] = {
	{ .compatible = "sandbox,eth" },
	{ }
};

U_BOOT_DRIVER(eth_sandbox) = {
	.name	= "eth_sandbox",
	.id	= UCLASS_ETH,
	.of_match = sb_eth_ids,
	.ofdata_to_platdata = sb_eth_ofdata_to_platdata,
	.remove	= sb_eth_remove,
	.ops	= &sb_eth_ops,
	.priv_auto_alloc_size = sizeof(struct eth_sandbox_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
