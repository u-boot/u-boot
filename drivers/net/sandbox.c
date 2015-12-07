/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
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
 * recv_packet_buffer: buffer of the packet returned as received
 * recv_packet_length: length of the packet returned as received
 */
struct eth_sandbox_priv {
	uchar fake_host_hwaddr[ARP_HLEN];
	struct in_addr fake_host_ipaddr;
	uchar *recv_packet_buffer;
	int recv_packet_length;
};

static bool disabled[8] = {false};
static bool skip_timeout;

/*
 * sandbox_eth_disable_response()
 *
 * index - The alias index (also DM seq number)
 * disable - If non-zero, ignore sent packets and don't send mock response
 */
void sandbox_eth_disable_response(int index, bool disable)
{
	disabled[index] = disable;
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

static int sb_eth_start(struct udevice *dev)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);

	debug("eth_sandbox: Start\n");

	fdtdec_get_byte_array(gd->fdt_blob, dev->of_offset, "fake-host-hwaddr",
			      priv->fake_host_hwaddr, ARP_HLEN);
	priv->recv_packet_buffer = net_rx_packets[0];
	return 0;
}

static int sb_eth_send(struct udevice *dev, void *packet, int length)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth = packet;

	debug("eth_sandbox: Send packet %d\n", length);

	if (dev->seq >= 0 && dev->seq < ARRAY_SIZE(disabled) &&
	    disabled[dev->seq])
		return 0;

	if (ntohs(eth->et_protlen) == PROT_ARP) {
		struct arp_hdr *arp = packet + ETHER_HDR_SIZE;

		if (ntohs(arp->ar_op) == ARPOP_REQUEST) {
			struct ethernet_hdr *eth_recv;
			struct arp_hdr *arp_recv;

			/* store this as the assumed IP of the fake host */
			priv->fake_host_ipaddr = net_read_ip(&arp->ar_tpa);
			/* Formulate a fake response */
			eth_recv = (void *)priv->recv_packet_buffer;
			memcpy(eth_recv->et_dest, eth->et_src, ARP_HLEN);
			memcpy(eth_recv->et_src, priv->fake_host_hwaddr,
			       ARP_HLEN);
			eth_recv->et_protlen = htons(PROT_ARP);

			arp_recv = (void *)priv->recv_packet_buffer +
				ETHER_HDR_SIZE;
			arp_recv->ar_hrd = htons(ARP_ETHER);
			arp_recv->ar_pro = htons(PROT_IP);
			arp_recv->ar_hln = ARP_HLEN;
			arp_recv->ar_pln = ARP_PLEN;
			arp_recv->ar_op = htons(ARPOP_REPLY);
			memcpy(&arp_recv->ar_sha, priv->fake_host_hwaddr,
			       ARP_HLEN);
			net_write_ip(&arp_recv->ar_spa, priv->fake_host_ipaddr);
			memcpy(&arp_recv->ar_tha, &arp->ar_sha, ARP_HLEN);
			net_copy_ip(&arp_recv->ar_tpa, &arp->ar_spa);

			priv->recv_packet_length = ETHER_HDR_SIZE +
				ARP_HDR_SIZE;
		}
	} else if (ntohs(eth->et_protlen) == PROT_IP) {
		struct ip_udp_hdr *ip = packet + ETHER_HDR_SIZE;

		if (ip->ip_p == IPPROTO_ICMP) {
			struct icmp_hdr *icmp = (struct icmp_hdr *)&ip->udp_src;

			if (icmp->type == ICMP_ECHO_REQUEST) {
				struct ethernet_hdr *eth_recv;
				struct ip_udp_hdr *ipr;
				struct icmp_hdr *icmpr;

				/* reply to the ping */
				memcpy(priv->recv_packet_buffer, packet,
				       length);
				eth_recv = (void *)priv->recv_packet_buffer;
				ipr = (void *)priv->recv_packet_buffer +
					ETHER_HDR_SIZE;
				icmpr = (struct icmp_hdr *)&ipr->udp_src;
				memcpy(eth_recv->et_dest, eth->et_src,
				       ARP_HLEN);
				memcpy(eth_recv->et_src, priv->fake_host_hwaddr,
				       ARP_HLEN);
				ipr->ip_sum = 0;
				ipr->ip_off = 0;
				net_copy_ip((void *)&ipr->ip_dst, &ip->ip_src);
				net_write_ip((void *)&ipr->ip_src,
					     priv->fake_host_ipaddr);
				ipr->ip_sum = compute_ip_checksum(ipr,
					IP_HDR_SIZE);

				icmpr->type = ICMP_ECHO_REPLY;
				icmpr->checksum = 0;
				icmpr->checksum = compute_ip_checksum(icmpr,
					ICMP_HDR_SIZE);

				priv->recv_packet_length = length;
			}
		}
	}

	return 0;
}

static int sb_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);

	if (skip_timeout) {
		sandbox_timer_add_offset(10000UL);
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

	pdata->iobase = dev_get_addr(dev);
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
