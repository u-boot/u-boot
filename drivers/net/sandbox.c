// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 */

#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <asm/eth.h>
#include <asm/global_data.h>
#include <asm/test.h>
#include <asm/types.h>

/*
 * Structure definitions for network protocols. Since this file is used for
 * both NET and NET_LWIP, and given that the two network stacks do have
 * conflicting types (for instance struct icmp_hdr), it is on purpose that the
 * structures are defined locally with minimal dependencies -- <asm/types.h> is
 * included for the bit types and that's it.
 */

#define ETHADDR_LEN 6
#define IP4_LEN 4

struct ethhdr {
        u8 dst[ETHADDR_LEN];
        u8 src[ETHADDR_LEN];
        u16 protlen;
} __attribute__((packed));

#define ETHHDR_SIZE (sizeof(struct ethhdr))

struct arphdr {
	u16 htype;
	u16 ptype;
	u8 hlen;
	u8 plen;
	u16 op;
} __attribute__((packed));

#define ARPHDR_SIZE (sizeof(struct arphdr))

#define ARP_REQUEST	1
#define ARP_REPLY	2

struct arpdata {
	u8 sha[ETHADDR_LEN];
	u32 spa;
	u8 tha[ETHADDR_LEN];
	u32 tpa;
} __attribute__((packed));

#define ARPDATA_SIZE (sizeof(struct arpdata))

struct iphdr {
	u8 hl_v;
	u8 tos;
	u16 len;
	u16 id;
	u16 off;
	u8 ttl;
	u8 prot;
	u16 sum;
	u32 src;
	u32 dst;
} __attribute__((packed));

#define IPHDR_SIZE (sizeof(struct iphdr))

struct icmphdr {
	u8 type;
	u8 code;
	u16 checksum;
	u16 id;
	u16 sequence;
} __attribute__((packed));

#define ICMPHDR_SIZE (sizeof(struct icmphdr))

#define ICMP_ECHO_REQUEST	8
#define ICMP_ECHO_REPLY		0
#define IPPROTO_ICMP		1

DECLARE_GLOBAL_DATA_PTR;

static const u8 null_ethaddr[6];
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
	struct ethhdr *eth = packet;
	struct arphdr *arp;
	struct arpdata *arpd;
	struct ethhdr *eth_recv;
	struct arphdr *arp_recv;
	struct arpdata *arp_recvd;

	if (ntohs(eth->protlen) != PROT_ARP)
		return -EAGAIN;

	arp = packet + ETHHDR_SIZE;

	if (ntohs(arp->op) != ARP_REQUEST)
		return -EAGAIN;

	/* Don't allow the buffer to overrun */
	if (priv->recv_packets >= PKTBUFSRX)
		return 0;

	/* store this as the assumed IP of the fake host */
	arpd = (struct arpdata *)(arp + 1);
	priv->fake_host_ipaddr.s_addr = arpd->tpa;

	/* Formulate a fake response */
	eth_recv = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(eth_recv->dst, eth->src, ETHADDR_LEN);
	memcpy(eth_recv->src, priv->fake_host_hwaddr, ETHADDR_LEN);
	eth_recv->protlen = htons(PROT_ARP);

	arp_recv = (void *)eth_recv + ETHHDR_SIZE;
	arp_recv->htype = htons(ARP_ETHER);
	arp_recv->ptype = htons(PROT_IP);
	arp_recv->hlen = ETHADDR_LEN;
	arp_recv->plen = IP4_LEN;
	arp_recv->op = htons(ARP_REPLY);
	arp_recvd = (struct arpdata *)(arp_recv + 1);
	memcpy(&arp_recvd->sha, priv->fake_host_hwaddr, ETHADDR_LEN);
	arp_recvd->spa = priv->fake_host_ipaddr.s_addr;
	memcpy(&arp_recvd->tha, &arpd->sha, ETHADDR_LEN);
	arp_recvd->tpa = arpd->spa;

	priv->recv_packet_length[priv->recv_packets] = ETHHDR_SIZE +
		ARPHDR_SIZE + ARPDATA_SIZE;
	++priv->recv_packets;

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
	struct ethhdr *eth = packet;
	struct iphdr *ip;
	struct icmphdr *icmp;
	struct ethhdr *eth_recv;
	struct iphdr *ipr;
	struct icmphdr *icmpr;

	if (ntohs(eth->protlen) != PROT_IP)
		return -EAGAIN;

	ip = packet + ETHHDR_SIZE;

	if (ip->prot != IPPROTO_ICMP)
		return -EAGAIN;

	icmp = (struct icmphdr *)(ip + 1);

	if (icmp->type != ICMP_ECHO_REQUEST)
		return -EAGAIN;

	/* Don't allow the buffer to overrun */
	if (priv->recv_packets >= PKTBUFSRX)
		return 0;

	/* reply to the ping */
	eth_recv = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(eth_recv, packet, len);
	ipr = (void *)eth_recv + ETHHDR_SIZE;
	icmpr = (struct icmphdr *)(ipr + 1);
	memcpy(eth_recv->dst, eth->src, ETHADDR_LEN);
	memcpy(eth_recv->src, priv->fake_host_hwaddr, ETHADDR_LEN);
	ipr->sum = 0;
	ipr->off = 0;
	ipr->dst = ip->src;
	ipr->src = priv->fake_host_ipaddr.s_addr;
	ipr->sum = compute_ip_checksum(ipr, IPHDR_SIZE);

	icmpr->type = ICMP_ECHO_REPLY;
	icmpr->checksum = 0;
	icmpr->checksum = compute_ip_checksum(icmpr, ICMPHDR_SIZE);

	priv->recv_packet_length[priv->recv_packets] = len;
	++priv->recv_packets;

	return 0;
}

/*
 * sandbox_eth_recv_arp_req()
 *
 * Inject an ARP request for this target
 *
 * returns 0 if injected, -EOVERFLOW if not
 */
int sandbox_eth_recv_arp_req(struct udevice *dev)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethhdr *eth_recv;
	struct arphdr *arp_recv;
	struct arpdata *arp_recvd;

	/* Don't allow the buffer to overrun */
	if (priv->recv_packets >= PKTBUFSRX)
		return -EOVERFLOW;

	/* Formulate a fake request */
	eth_recv = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(eth_recv->dst, net_bcast_ethaddr, ETHADDR_LEN);
	memcpy(eth_recv->src, priv->fake_host_hwaddr, ETHADDR_LEN);
	eth_recv->protlen = htons(PROT_ARP);

	arp_recv = (void *)eth_recv + ETHHDR_SIZE;
	arp_recv->htype = htons(ARP_ETHER);
	arp_recv->ptype = htons(PROT_IP);
	arp_recv->hlen = ETHADDR_LEN;
	arp_recv->plen = IP4_LEN;
	arp_recv->op = htons(ARP_REQUEST);
	arp_recvd = (struct arpdata *)(arp_recv + 1);
	memcpy(&arp_recvd->sha, priv->fake_host_hwaddr, ETHADDR_LEN);
	arp_recvd->spa = priv->fake_host_ipaddr.s_addr;
	memcpy(&arp_recvd->tha, null_ethaddr, ETHADDR_LEN);
	arp_recvd->tpa = net_ip.s_addr;

	priv->recv_packet_length[priv->recv_packets] =
		ETHHDR_SIZE + ARPHDR_SIZE + ARPDATA_SIZE;
	++priv->recv_packets;

	return 0;
}

/*
 * sandbox_eth_recv_ping_req()
 *
 * Inject a ping request for this target
 *
 * returns 0 if injected, -EOVERFLOW if not
 */
int sandbox_eth_recv_ping_req(struct udevice *dev)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct ethhdr *eth_recv;
	struct iphdr *ipr;
	struct icmphdr *icmpr;

	/* Don't allow the buffer to overrun */
	if (priv->recv_packets >= PKTBUFSRX)
		return -EOVERFLOW;

	/* Formulate a fake ping */
	eth_recv = (void *)priv->recv_packet_buffer[priv->recv_packets];

	memcpy(eth_recv->dst, pdata->enetaddr, ETHADDR_LEN);
	memcpy(eth_recv->src, priv->fake_host_hwaddr, ETHADDR_LEN);
	eth_recv->protlen = htons(PROT_IP);

	ipr = (void *)eth_recv + ETHHDR_SIZE;
	ipr->hl_v = 0x45;
	ipr->len = htons(IPHDR_SIZE + ICMPHDR_SIZE);
	ipr->off = htons(IP_FLAGS_DFRAG);
	ipr->prot = IPPROTO_ICMP;
	ipr->sum = 0;
	ipr->src = priv->fake_host_ipaddr.s_addr;
	ipr->dst = net_ip.s_addr;
	ipr->sum = compute_ip_checksum(ipr, IPHDR_SIZE);

	icmpr = (struct icmphdr *)(ipr + 1);

	icmpr->type = ICMP_ECHO_REQUEST;
	icmpr->code = 0;
	icmpr->checksum = 0;
	icmpr->id = 0;
	icmpr->sequence = htons(1);
	icmpr->checksum = compute_ip_checksum(icmpr, ICMPHDR_SIZE);

	priv->recv_packet_length[priv->recv_packets] =
		ETHHDR_SIZE + IPHDR_SIZE + ICMPHDR_SIZE;
	++priv->recv_packets;

	return 0;
}

/*
 * sb_default_handler()
 *
 * perform typical responses to simple ping
 *
 * dev - device pointer
 * pkt - "sent" packet buffer
 * len - length of packet
 */
static int sb_default_handler(struct udevice *dev, void *packet,
			      unsigned int len)
{
	if (!sandbox_eth_arp_req_to_reply(dev, packet, len))
		return 0;
	if (!sandbox_eth_ping_req_to_reply(dev, packet, len))
		return 0;

	return 0;
}

/*
 * sandbox_eth_set_tx_handler()
 *
 * Set a custom response to a packet being sent through the sandbox eth test
 *	driver
 *
 * index - interface to set the handler for
 * handler - The func ptr to call on send. If NULL, set to default handler
 */
void sandbox_eth_set_tx_handler(int index, sandbox_eth_tx_hand_f *handler)
{
	struct udevice *dev;
	struct eth_sandbox_priv *priv;
	int ret;

	ret = uclass_get_device(UCLASS_ETH, index, &dev);
	if (ret)
		return;

	priv = dev_get_priv(dev);
	if (handler)
		priv->tx_handler = handler;
	else
		priv->tx_handler = sb_default_handler;
}

/*
 * Set priv ptr
 *
 * priv - priv void ptr to store in the device
 */
void sandbox_eth_set_priv(int index, void *priv)
{
	struct udevice *dev;
	struct eth_sandbox_priv *dev_priv;
	int ret;

	ret = uclass_get_device(UCLASS_ETH, index, &dev);
	if (ret)
		return;

	dev_priv = dev_get_priv(dev);

	dev_priv->priv = priv;
}

static int sb_eth_start(struct udevice *dev)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);

	debug("eth_sandbox: Start\n");

	priv->recv_packets = 0;
	for (int i = 0; i < PKTBUFSRX; i++) {
		priv->recv_packet_buffer[i] = net_rx_packets[i];
		priv->recv_packet_length[i] = 0;
	}

	return 0;
}

static int sb_eth_send(struct udevice *dev, void *packet, int length)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);

	debug("eth_sandbox: Send packet %d\n", length);

	if (priv->disabled)
		return 0;

	return priv->tx_handler(dev, packet, length);
}

static int sb_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);

	if (skip_timeout) {
		timer_test_add_offset(11000UL);
		skip_timeout = false;
	}

	if (priv->recv_packets) {
		int lcl_recv_packet_length = priv->recv_packet_length[0];

		debug("eth_sandbox: received packet[%d], %d waiting\n",
		      lcl_recv_packet_length, priv->recv_packets - 1);
		*packetp = priv->recv_packet_buffer[0];
		return lcl_recv_packet_length;
	}
	return 0;
}

static int sb_eth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	int i;

	if (!priv->recv_packets)
		return 0;

	--priv->recv_packets;
	for (i = 0; i < priv->recv_packets; i++) {
		priv->recv_packet_length[i] = priv->recv_packet_length[i + 1];
		memcpy(priv->recv_packet_buffer[i],
		       priv->recv_packet_buffer[i + 1],
		       priv->recv_packet_length[i + 1]);
	}
	priv->recv_packet_length[priv->recv_packets] = 0;

	return 0;
}

static void sb_eth_stop(struct udevice *dev)
{
	debug("eth_sandbox: Stop\n");
}

static int sb_eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct eth_sandbox_priv *priv = dev_get_priv(dev);

	debug("eth_sandbox %s: Write HW ADDR - %pM\n", dev->name,
	      pdata->enetaddr);
	memcpy(priv->fake_host_hwaddr, pdata->enetaddr, ETHADDR_LEN);
	return 0;
}

static const struct eth_ops sb_eth_ops = {
	.start			= sb_eth_start,
	.send			= sb_eth_send,
	.recv			= sb_eth_recv,
	.free_pkt		= sb_eth_free_pkt,
	.stop			= sb_eth_stop,
	.write_hwaddr		= sb_eth_write_hwaddr,
};

static int sb_eth_remove(struct udevice *dev)
{
	return 0;
}

static int sb_eth_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct eth_sandbox_priv *priv = dev_get_priv(dev);

	pdata->iobase = dev_read_addr(dev);
	priv->disabled = false;
	priv->tx_handler = sb_default_handler;

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
	.of_to_plat = sb_eth_of_to_plat,
	.remove	= sb_eth_remove,
	.ops	= &sb_eth_ops,
	.priv_auto	= sizeof(struct eth_sandbox_priv),
	.plat_auto	= sizeof(struct eth_pdata),
};
