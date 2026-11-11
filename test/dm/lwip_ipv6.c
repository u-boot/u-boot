// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2026 James Hilliard <james.hilliard1@gmail.com> */

#include <command.h>
#include <console.h>
#include <dm.h>
#include <env.h>
#include <image.h>
#include <lmb.h>
#include <malloc.h>
#include <lwip/inet_chksum.h>
#include <lwip/ip6.h>
#include <lwip/nd6.h>
#include <lwip/dns.h>
#include <lwip/timeouts.h>
#include <lwip/tcp.h>
#include <lwip/udp.h>
#include <lwip-hooks.h>
#include <lwip/priv/nd6_priv.h>
#include <lwip/prot/icmp6.h>
#include <lwip/prot/nd6.h>
#include <lwip/prot/udp.h>
#include <mapmem.h>
#include <net.h>
#include <stdio_dev.h>
#include <time.h>
#include <asm/eth.h>
#include <asm/unaligned.h>
#include <dm/test.h>
#include <test/ut.h>

struct ip6_test {
	struct net_lwip_ctx net;
	unsigned int dad;
	unsigned int neighbors;
	ip6_addr_t last_neighbor;
	unsigned int solicitations;
	unsigned int echoes;
	unsigned int bad_checksums;
	unsigned int corrupt;
	unsigned int requests;
	unsigned int uploads;
	unsigned int fragments;
	unsigned int dns_queries;
	unsigned int dns4_queries;
	bool drop_dns;
	ip6_addr_t last_dns;
	unsigned int http_requests;
	unsigned int rpc_requests;
	unsigned int sntp_requests;
	unsigned int console_packets;
	bool console_no_reply;
	const char *http_host;
	const char *http_target;
	const char *tftp_file;
	bool unreachable;
	bool listener;
	bool listener_done;
	u32 rdnss_lifetime;
	bool lease_conflict;
	bool conflict;
	bool advertise;
	bool cancel;
	bool fragment;
};

static u16 ip6_test_checksum(struct ip6_hdr *ip, void *data, unsigned int len)
{
	struct pbuf p = { .payload = data, .len = len, .tot_len = len };
	ip6_addr_t src, dst;

	ip6_addr_copy_from_packed(src, ip->src);
	ip6_addr_copy_from_packed(dst, ip->dest);
	return ip6_chksum_pseudo(&p, IP6H_NEXTH(ip), len, &src, &dst);
}

static int ip6_test_reply(struct udevice *dev, const ip6_addr_t *src,
			  const ip6_addr_t *dst, const void *payload, unsigned int len,
			  u8 proto, bool bad_checksum)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ip6_test *test = priv->priv;
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct ethernet_hdr *eth;
	struct icmp6_hdr *icmp;
	struct udp_hdr *udp;
	struct ip6_hdr *ip;
	unsigned int size = ETHER_HDR_SIZE + IP6_HLEN + len;

	if (priv->recv_packets >= PKTBUFSRX || size > PKTSIZE_ALIGN)
		return -ENOSPC;
	eth = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memset(eth, 0, size);
	memcpy(eth->et_dest, pdata->enetaddr, ARP_HLEN);
	memcpy(eth->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	eth->et_protlen = htons(0x86dd);
	ip = (void *)(eth + 1);
	IP6H_VTCFL_SET(ip, 6, 0, 0);
	IP6H_PLEN_SET(ip, len);
	IP6H_NEXTH_SET(ip, proto);
	IP6H_HOPLIM_SET(ip, 255);
	ip6_addr_copy_to_packed(ip->src, *src);
	ip6_addr_copy_to_packed(ip->dest, *dst);
	icmp = (void *)(ip + 1);
	memcpy(icmp, payload, len);
	if (proto == IP6_NEXTH_UDP) {
		udp = (void *)icmp;
		udp->chksum = 0;
		udp->chksum = ip6_test_checksum(ip, udp, len);
		if (!udp->chksum)
			udp->chksum = 0xffff;
	} else if (proto == IP6_NEXTH_TCP) {
		put_unaligned(0, (u16 *)((u8 *)icmp + 16));
		put_unaligned(ip6_test_checksum(ip, icmp, len), (u16 *)((u8 *)icmp + 16));
	} else {
		icmp->chksum = 0;
		icmp->chksum = ip6_test_checksum(ip, icmp, len);
		if (bad_checksum)
			icmp->chksum ^= htons(1);
	}
	if (proto == IP6_NEXTH_UDP && test->fragment) {
		struct ethernet_hdr *tail;
		struct ip6_hdr *tail_ip;
		struct ip6_frag_hdr *first_frag = (void *)(ip + 1), *last_frag;
		u32 id = htonl(++test->fragments);

		if (priv->recv_packets + 1 >= PKTBUFSRX || len <= UDP_HLEN)
			return -ENOSPC;
		tail = (void *)priv->recv_packet_buffer[priv->recv_packets + 1];
		memcpy(tail, eth, ETHER_HDR_SIZE + IP6_HLEN);
		tail_ip = (void *)(tail + 1);
		last_frag = (void *)(tail_ip + 1);
		memcpy(last_frag + 1, (u8 *)icmp + UDP_HLEN, len - UDP_HLEN);
		*last_frag = (struct ip6_frag_hdr) {
			._nexth = IP6_NEXTH_UDP,
			._fragment_offset = htons(UDP_HLEN),
			._identification = id,
		};
		IP6H_NEXTH_SET(tail_ip, IP6_NEXTH_FRAGMENT);
		IP6H_PLEN_SET(tail_ip, IP6_FRAG_HLEN + len - UDP_HLEN);
		priv->recv_packet_length[priv->recv_packets + 1] = size;
		memmove(first_frag + 1, icmp, UDP_HLEN);
		*first_frag = (struct ip6_frag_hdr) {
			._nexth = IP6_NEXTH_UDP,
			._fragment_offset = htons(IP6_FRAG_MORE_FLAG),
			._identification = id,
		};
		IP6H_NEXTH_SET(ip, IP6_NEXTH_FRAGMENT);
		IP6H_PLEN_SET(ip, IP6_FRAG_HLEN + UDP_HLEN);
		priv->recv_packet_length[priv->recv_packets] =
			ETHER_HDR_SIZE + IP6_HLEN + IP6_FRAG_HLEN + UDP_HLEN;
		priv->recv_packets += 2;
		return 0;
	}
	priv->recv_packet_length[priv->recv_packets++] = size;

	return 0;
}

static int ip6_test_na(struct udevice *dev, struct ns_header *ns,
		       const ip6_addr_t *dest, bool dad)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct {
		struct na_header na;
		struct lladdr_option ll;
	} reply = {};
	ip6_addr_t src, dst = *dest;

	ip6_addr_copy_from_packed(src, ns->target_address);
	if (dad)
		ip6addr_aton("ff02::1", &dst);
	reply.na.type = ICMP6_TYPE_NA;
	reply.na.flags = ND6_FLAG_OVERRIDE | (dad ? 0 : ND6_FLAG_SOLICITED);
	reply.na.target_address = ns->target_address;
	reply.ll.type = ND6_OPTION_TYPE_TARGET_LLADDR;
	reply.ll.length = 1;
	memcpy(reply.ll.addr, priv->fake_host_hwaddr, ARP_HLEN);

	return ip6_test_reply(dev, &src, &dst, &reply, sizeof(reply), IP6_NEXTH_ICMP6, false);
}

static int ip6_test_ra_lifetimes(struct udevice *dev, u32 valid, u32 preferred)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct {
		struct ra_header ra;
		struct prefix_option prefix;
		struct lladdr_option ll;
		struct rdnss_option dns;
	} reply = {};
	ip6_addr_t src, dst, prefix;

	ip6addr_aton("fe80::1", &src);
	ip6addr_aton("ff02::1", &dst);
	ip6addr_aton("2001:db8:1::", &prefix);
	reply.ra.type = ICMP6_TYPE_RA;
	reply.ra.current_hop_limit = 64;
	reply.ra.router_lifetime = htons(1800);
	reply.prefix.type = ND6_OPTION_TYPE_PREFIX_INFO;
	reply.prefix.length = 4;
	reply.prefix.prefix_length = 64;
	reply.prefix.flags = ND6_PREFIX_FLAG_ON_LINK | ND6_PREFIX_FLAG_AUTONOMOUS;
	reply.prefix.valid_lifetime = htonl(valid);
	reply.prefix.preferred_lifetime = htonl(preferred);
	ip6_addr_copy_to_packed(reply.prefix.prefix, prefix);
	reply.ll.type = ND6_OPTION_TYPE_SOURCE_LLADDR;
	reply.ll.length = 1;
	memcpy(reply.ll.addr, priv->fake_host_hwaddr, ARP_HLEN);
	reply.dns.type = ND6_OPTION_TYPE_RDNSS;
	reply.dns.length = 3;
	reply.dns.lifetime = htonl(((struct ip6_test *)priv->priv)->rdnss_lifetime);
	ip6_addr_copy_to_packed(reply.dns.rdnss_address[0], src);

	return ip6_test_reply(dev, &src, &dst, &reply, sizeof(reply), IP6_NEXTH_ICMP6, false);
}

static int ip6_test_ra(struct udevice *dev)
{
	return ip6_test_ra_lifetimes(dev, 1800, 900);
}

static int ip6_test_tftp_reply(struct udevice *dev, struct ip6_hdr *ip, unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ip6_test *test = priv->priv;
	struct udp_hdr *udp = (void *)(ip + 1);
	struct {
		struct udp_hdr udp;
		u8 data[4 + sizeof("ipv6-tftp") - 1];
	} __packed reply = {};
	ip6_addr_t src, dst;
	u8 *data = (void *)(udp + 1);
	unsigned int size = UDP_HLEN + 4;
	u16 op;

	if (len < IP6_HLEN + UDP_HLEN + 4 || IP6H_PLEN(ip) < UDP_HLEN + 4)
		return -EINVAL;
	if (ip6_test_checksum(ip, udp, IP6H_PLEN(ip)))
		test->bad_checksums++;
	op = get_unaligned_be16(data);
	if (op == 1 || op == 2) {
		test->requests++;
		if (strcmp((char *)data + 2, test->tftp_file ?: "ipv6.bin"))
			return -EINVAL;
		put_unaligned_be16(op == 1 ? 3 : 4, reply.data);
		put_unaligned_be16(op == 1 ? 1 : 0, reply.data + 2);
		if (op == 1) {
			memcpy(reply.data + 4, "ipv6-tftp", 9);
			size += 9;
		}
	} else if (op == 3) {
		if (IP6H_PLEN(ip) != UDP_HLEN + 4 + 9 ||
		    memcmp(data + 4, "ipv6-tftp", 9))
			return -EINVAL;
		test->uploads++;
		put_unaligned_be16(4, reply.data);
		put_unaligned_be16(1, reply.data + 2);
	} else if (op == 4 && test->listener) {
		if (get_unaligned_be16(data + 2) == 1) {
			test->listener_done = true;
			return 0;
		}
		put_unaligned_be16(3, reply.data);
		put_unaligned_be16(1, reply.data + 2);
		memcpy(reply.data + 4, "ipv6-tftp", 9);
		size += 9;
	} else {
		return 0;
	}
	reply.udp.src = htons(1234);
	reply.udp.dest = udp->src;
	reply.udp.len = htons(size);
	ip6_addr_copy_from_packed(src, ip->dest);
	ip6_addr_copy_from_packed(dst, ip->src);

	return ip6_test_reply(dev, &src, &dst, &reply, size, IP6_NEXTH_UDP, false);
}

static int ip6_test_udp(struct udevice *dev, struct ip6_hdr *ip, u16 port,
			const void *data, unsigned int len)
{
	u8 packet[1200];
	struct udp_hdr *udp = (void *)packet, *request = (void *)(ip + 1);
	ip6_addr_t src, dst;

	if (len > sizeof(packet) - sizeof(*udp))
		return -ENOSPC;
	udp->src = htons(port);
	udp->dest = request->src;
	udp->len = htons(sizeof(*udp) + len);
	memcpy(udp + 1, data, len);
	ip6_addr_copy_from_packed(src, ip->dest);
	if (ip6_addr_ismulticast(&src))
		ip6addr_aton("fe80::2", &src);
	ip6_addr_copy_from_packed(dst, ip->src);
	return ip6_test_reply(dev, &src, &dst, packet, sizeof(*udp) + len, IP6_NEXTH_UDP, false);
}

static int ip6_test_dns_answer(struct ip6_test *test, const u8 *query,
			       unsigned int len, u8 *reply, size_t size)
{
	ip6_addr_t addr;
	u8 *answer;

	if (len < 16 || len > size - 28)
		return -EINVAL;
	/* These tests explicitly select AAAA and IN. */
	if (get_unaligned_be16(query + len - 4) != 28 || get_unaligned_be16(query + len - 2) != 1)
		return -EINVAL;
	test->dns_queries++;
	memcpy(reply, query, len);
	put_unaligned_be16(0x8180, reply + 2);
	put_unaligned_be16(1, reply + 6);
	answer = reply + len;
	put_unaligned_be16(0xc00c, answer);
	put_unaligned_be16(28, answer + 2);
	put_unaligned_be16(1, answer + 4);
	put_unaligned_be32(30, answer + 6);
	put_unaligned_be16(16, answer + 10);
	ip6addr_aton("fe80::2", &addr);
	memcpy(answer + 12, addr.addr, 16);
	return len + 28;
}

static int ip6_test_dns_reply(struct udevice *dev, struct ip6_hdr *ip)
{
	struct ip6_test *test = ((struct eth_sandbox_priv *)dev_get_priv(dev))->priv;
	u8 *query = (void *)((struct udp_hdr *)(ip + 1) + 1), reply[512];
	int len;

	if (test->drop_dns) {
		test->dns_queries++;
		timer_test_add_offset(11000);
		return 0;
	}
	ip6_addr_copy_from_packed(test->last_dns, ip->dest);
	len = ip6_test_dns_answer(test, query, IP6H_PLEN(ip) - UDP_HLEN,
				  reply, sizeof(reply));
	return len < 0 ? len : ip6_test_udp(dev, ip, 53, reply, len);
}

static int ip6_test_ipv4(struct udevice *dev, void *packet, unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ip6_test *test = priv->priv;
	struct ethernet_hdr *eth = packet, *reply_eth;
	struct ip_udp_hdr *ip = (void *)(eth + 1), *reply_ip;
	int size;

	if (!sandbox_eth_arp_req_to_reply(dev, packet, len) ||
	    !sandbox_eth_ping_req_to_reply(dev, packet, len))
		return 0;
	if (ntohs(eth->et_protlen) != PROT_IP || len < ETHER_HDR_SIZE + IP_UDP_HDR_SIZE ||
	    ip->ip_hl_v != 0x45 || ip->ip_p != IPPROTO_UDP || ntohs(ip->udp_dst) != 53)
		return 0;
	if (priv->recv_packets >= PKTBUFSRX || ntohs(ip->udp_len) < UDP_HDR_SIZE ||
	    ntohs(ip->udp_len) > len - ETHER_HDR_SIZE - IP_HDR_SIZE)
		return -EINVAL;
	reply_eth = (void *)priv->recv_packet_buffer[priv->recv_packets];
	reply_ip = (void *)(reply_eth + 1);
	size = ip6_test_dns_answer(test, (void *)(ip + 1), ntohs(ip->udp_len) - UDP_HDR_SIZE,
				   (void *)(reply_ip + 1), 512);
	if (size < 0)
		return size;
	memcpy(reply_eth, eth, ETHER_HDR_SIZE + IP_UDP_HDR_SIZE);
	memcpy(reply_eth->et_dest, eth->et_src, ARP_HLEN);
	memcpy(reply_eth->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	reply_ip->ip_src = ip->ip_dst;
	reply_ip->ip_dst = ip->ip_src;
	reply_ip->ip_len = htons(IP_UDP_HDR_SIZE + size);
	reply_ip->ip_sum = 0;
	reply_ip->ip_sum = compute_ip_checksum(reply_ip, IP_HDR_SIZE);
	reply_ip->udp_src = ip->udp_dst;
	reply_ip->udp_dst = ip->udp_src;
	reply_ip->udp_len = htons(UDP_HDR_SIZE + size);
	reply_ip->udp_xsum = 0;
	priv->recv_packet_length[priv->recv_packets++] = ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + size;
	test->dns4_queries++;
	return 0;
}

static int ip6_test_http_reply(struct udevice *dev, struct ip6_hdr *ip)
{
	struct ip6_test *test = ((struct eth_sandbox_priv *)dev_get_priv(dev))->priv;
	static const char response[] = "HTTP/1.1 200 OK\r\nContent-Length: 9\r\n"
				       "Connection: close\r\n\r\nipv6-http";
	u8 *tcp = (void *)(ip + 1), packet[20 + sizeof(response)] = {};
	ip6_addr_t src, dst;
	unsigned int header = (tcp[12] >> 4) * 4, len = IP6H_PLEN(ip), size = 20;
	u32 seq = get_unaligned_be32(tcp + 4);

	if (len < 20 || header < 20 || header > len)
		return -EINVAL;
	if (ip6_test_checksum(ip, tcp, len))
		test->bad_checksums++;
	memcpy(packet, tcp + 2, 2);
	memcpy(packet + 2, tcp, 2);
	packet[12] = 5 << 4;
	put_unaligned_be16(32768, packet + 14);
	if (tcp[13] & 2) {
		put_unaligned_be32(1000, packet + 4);
		put_unaligned_be32(seq + 1, packet + 8);
		packet[13] = 0x12;
	} else if (len > header) {
		if (!test->http_host ||
		    !strnstr((char *)tcp + header, test->http_host, len - header) ||
		    (test->http_target &&
		     !strnstr((char *)tcp + header, test->http_target, len - header)))
			return -EINVAL;
		test->http_requests++;
		put_unaligned_be32(1001, packet + 4);
		put_unaligned_be32(seq + len - header, packet + 8);
		packet[13] = 0x19;
		memcpy(packet + 20, response, sizeof(response) - 1);
		size += sizeof(response) - 1;
	} else {
		return 0;
	}
	ip6_addr_copy_from_packed(src, ip->dest);
	ip6_addr_copy_from_packed(dst, ip->src);
	return ip6_test_reply(dev, &src, &dst, packet, size, IP6_NEXTH_TCP, false);
}

static int ip6_test_rpc_reply(struct udevice *dev, struct ip6_hdr *ip)
{
	struct ip6_test *test = ((struct eth_sandbox_priv *)dev_get_priv(dev))->priv;
	struct udp_hdr *udp = (void *)(ip + 1);
	u32 reply[288] = {};
	u8 *query = (void *)(udp + 1);
	const char *uaddr;
	int ret;

	memcpy(reply, query, 4);
	reply[1] = htonl(1);
	switch (++test->rpc_requests) {
	case 1:
	case 2:
		if (get_unaligned_be32(query + 12) != 100000 ||
		    get_unaligned_be32(query + 16) != 3 ||
		    get_unaligned_be32(query + 20) != 3 ||
		    get_unaligned_be32(query + 48) != 4 || memcmp(query + 52, "udp6", 4))
			return -EINVAL;
		/* A truncated universal address must not advance service discovery. */
		reply[6] = htonl(100);
		ret = ip6_test_udp(dev, ip, ntohs(udp->dest), reply, 28);
		if (ret)
			return ret;
		uaddr = test->rpc_requests == 1 ? "fe80::2.15.160" : "fe80::2.15.161";
		reply[6] = htonl(strlen(uaddr));
		memcpy(reply + 7, uaddr, strlen(uaddr));
		break;
	case 3:
	case 4:
		reply[7] = htonl(32);
		break;
	case 5:
		reply[8] = htonl(9);
		reply[9] = htonl(1);
		reply[10] = htonl(9);
		memcpy(reply + 11, "ipv6-nfs!", 9);
		break;
	case 6:
	case 7:
		break;
	default:
		return -EINVAL;
	}
	return ip6_test_udp(dev, ip, ntohs(udp->dest), reply, sizeof(reply));
}

static int ip6_test_sntp_reply(struct udevice *dev, struct ip6_hdr *ip)
{
	struct ip6_test *test = ((struct eth_sandbox_priv *)dev_get_priv(dev))->priv;
	u8 reply[48] = {}, *query = (void *)((struct udp_hdr *)(ip + 1) + 1);

	if (IP6H_PLEN(ip) != UDP_HLEN + sizeof(reply))
		return -EINVAL;
	reply[0] = 0x24;
	reply[1] = 1;
	memcpy(reply + 24, query + 40, 8);
	put_unaligned_be32(0xe0000000, reply + 32);
	put_unaligned_be32(0xe0000000, reply + 40);
	test->sntp_requests++;
	return ip6_test_udp(dev, ip, 123, reply, sizeof(reply));
}

static int ip6_test_tx(struct udevice *dev, void *packet, unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ip6_test *test = priv->priv;
	struct ethernet_hdr *eth = packet;
	struct ip6_hdr *ip = (void *)(eth + 1);
	struct icmp6_echo_hdr echo;
	struct ns_header *ns;
	ip6_addr_t src, dst;
	unsigned int plen;
	u8 *icmp = (void *)(ip + 1);

	if (len < ETHER_HDR_SIZE)
		return 0;
	if (ntohs(eth->et_protlen) != 0x86dd)
		return ip6_test_ipv4(dev, packet, len);
	if (len < ETHER_HDR_SIZE + IP6_HLEN)
		return 0;
	if (IP6H_NEXTH(ip) == IP6_NEXTH_UDP) {
		struct udp_hdr *udp = (void *)(ip + 1);

		if (IP6H_PLEN(ip) < UDP_HLEN || len < ETHER_HDR_SIZE + IP6_HLEN + IP6H_PLEN(ip))
			return -EINVAL;
		if (test->unreachable) {
			u8 packet[sizeof(struct icmp6_hdr) + IP6_HLEN + UDP_HLEN] = {
				ICMP6_TYPE_DUR, ICMP6_DUR_PORT,
			};

			memcpy(packet + sizeof(struct icmp6_hdr), ip, IP6_HLEN + UDP_HLEN);
			ip6_addr_copy_from_packed(src, ip->dest);
			ip6_addr_copy_from_packed(dst, ip->src);
			return ip6_test_reply(dev, &src, &dst, packet, sizeof(packet),
					     IP6_NEXTH_ICMP6, false);
		}
		if (ntohs(udp->dest) == 53)
			return ip6_test_dns_reply(dev, ip);
		if (ntohs(udp->dest) == 111 || ntohs(udp->dest) == 4000 || ntohs(udp->dest) == 4001)
			return ip6_test_rpc_reply(dev, ip);
		if (ntohs(udp->dest) == 123)
			return ip6_test_sntp_reply(dev, ip);
		if (ntohs(udp->dest) == 6666) {
			test->console_packets++;
			if (test->console_no_reply)
				return 0;
			return ip6_test_udp(dev, ip, 6666, "x", 1);
		}
		return ip6_test_tftp_reply(dev, ip, len - ETHER_HDR_SIZE);
	}
	if (IP6H_NEXTH(ip) == IP6_NEXTH_TCP)
		return ip6_test_http_reply(dev, ip);
	if (IP6H_NEXTH(ip) != IP6_NEXTH_ICMP6)
		return 0;
	plen = IP6H_PLEN(ip);
	if (plen < sizeof(struct icmp6_hdr) || len < ETHER_HDR_SIZE + IP6_HLEN + plen)
		return -EINVAL;
	if (ip6_test_checksum(ip, icmp, plen))
		test->bad_checksums++;
	ip6_addr_copy_from_packed(src, ip->src);
	ip6_addr_copy_from_packed(dst, ip->dest);
	switch (*icmp) {
	case ICMP6_TYPE_NS:
		if (plen < sizeof(*ns))
			return -EINVAL;
		ns = (void *)icmp;
		if (ip6_addr_isany(&src)) {
			test->dad++;
			if (test->cancel)
				console_in_puts("\x03");
			if (test->conflict ||
			    (test->lease_conflict &&
			     !memcmp(&ns->target_address, "\x20\x01", 2)))
				return ip6_test_na(dev, ns, &src, true);
			timer_test_add_offset(1100);
			return 0;
		}
		test->neighbors++;
		ip6_addr_copy_from_packed(test->last_neighbor, ns->target_address);
		return ip6_test_na(dev, ns, &src, false);
	case ICMP6_TYPE_RS:
		test->solicitations++;
		if (test->advertise)
			return ip6_test_ra(dev);
		return 0;
	case ICMP6_TYPE_EREQ:
		test->echoes++;
		if (plen != sizeof(echo))
			return -EINVAL;
		memcpy(&echo, icmp, sizeof(echo));
		echo.type = ICMP6_TYPE_EREP;
		if (test->echoes == 1) {
			if (test->corrupt == 2)
				echo.type = ICMP6_TYPE_EREQ;
			if (test->corrupt == 3)
				echo.seqno ^= htons(1);
		}
		return ip6_test_reply(dev, &dst, &src, &echo, sizeof(echo),
				     IP6_NEXTH_ICMP6, test->echoes == 1 && test->corrupt == 1);
	default:
		return 0;
	}
}

static int ip6_test_config(struct unit_test_state *uts, struct ip6_test *test)
{
	static const char * const invalid[] = {
		"::", "::1", "ff02::1", "1.2.3.4", "2001:db8::1/129",
		"2001:db8::1/", "2001:db8::1/-1", "2001:db8::1/+1",
		"2001:db8::1/64junk", "2001:db8::1%et0", "2001:db8::1junk",
		"2001:db8::1::2", "2001:db8::12345", "::ffff:c000:201",
		"2001:db8:0:0:0:0:0:1:", ":2001:db8:0:0:0:0:0:1", "2001:db8:1",
		"2001:db8:0:0:0:0:1:", ":2001:db8:0:0:0:0:1", "2001:db8:::1",
		"2001:db8:0:0:0:0:0:1::", "::2001:db8:0:0:0:0:0:1",
		"2001:db8::10/18446744073709551680", "2001:db8::10/4294967360",
	};
	static const char *const valid[] = {
		"2001:db8:0:0:0:0:0:1", "2001:db8::1", "2001:db8:0:0:0:0:1::",
		"::2001:db8:0:0:0:0:1", "2001::", "2001:0:0:0:0:0:0:0",
	};
	struct net_lwip_ctx other = {};
	ip6_addr_t expected;
	ip4_addr_t ip4;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(valid); i++)
		ut_assertok(net_lwip_ip6_aton(valid[i], &expected));
	ut_assertok(net_lwip_start(&test->net, NET_LWIP_ADDR_ENV_FLEXIBLE));
	ip4 = *netif_ip4_addr(test->net.netif);
	for (i = 0; i < ARRAY_SIZE(invalid); i++) {
		ut_assertok(env_set("ip6addr", invalid[i]));
		ut_asserteq(-EINVAL, net_lwip_ip6_init(&test->net));
		ut_assert(ip6_addr_isinvalid(netif_ip6_addr_state(test->net.netif, 0)));
		ut_assert(ip4_addr_cmp(&ip4, netif_ip4_addr(test->net.netif)));
	}
	ut_assertok(env_set("ip6addr", "2001:db8:1::10/00064"));
	ut_assertok(env_set("gatewayip6", "ff02::1"));
	ut_asserteq(-EINVAL, net_lwip_ip6_init(&test->net));
	ut_assertok(env_set("gatewayip6", "fe80::1"));
	ut_assertok(net_lwip_ip6_init(&test->net));
	ip6addr_aton("2001:db8:1::10", &expected);
	ut_assert(ip6_addr_zoneless_eq(&expected, netif_ip6_addr(test->net.netif, 1)));
	ut_asserteq(IP6_ADDR_TENTATIVE, netif_ip6_addr_state(test->net.netif, 1));
	ut_assertok(net_lwip_ip6_init(&test->net));
	ut_assertok(env_set("ip6addr", "2001:db8:1::11"));
	ut_asserteq(-EBUSY, net_lwip_ip6_init(&test->net));
	ut_assert(ip6_addr_zoneless_eq(&expected, netif_ip6_addr(test->net.netif, 1)));
	ut_assertok(net_lwip_start(&other, NET_LWIP_ADDR_ENV_STRICT));
	net_lwip_stop(&other);
	ut_assert(ip4_addr_cmp(&ip4, netif_ip4_addr(test->net.netif)));
	net_lwip_stop(&test->net);
	ut_assertnull(netif_default);
	ut_assertok(net_lwip_start(&test->net, NET_LWIP_ADDR_ENV_FLEXIBLE));
	ut_assertok(net_lwip_ip6_init(&test->net));
	ip6addr_aton("2001:db8:1::11", &expected);
	ut_assert(ip6_addr_zoneless_eq(&expected, netif_ip6_addr(test->net.netif, 1)));

	return 0;
}

static int ip6_test_ping(struct unit_test_state *uts, struct ip6_test *test)
{
	unsigned int i;

	/* A corrupt reply must not complete a command; a later reply can. */
	for (i = 0; i < 4; i++) {
		test->echoes = 0;
		test->corrupt = i;
		ut_assertok(run_command("ping6 fe80::2", 0));
		ut_asserteq(i ? 2 : 1, test->echoes);
		ut_assertnull(netif_default);
	}
	ut_assert(test->dad >= 4);
	ut_assert(test->neighbors >= 4);
	ut_asserteq(0, test->bad_checksums);
	test->corrupt = 0;
	ut_assertok(env_set("ip6addr", "2001:db8:1::10/64"));
	ut_assertok(run_command("ping6 2001:db8:1::2", 0));
	ut_assertok(env_set("gatewayip6", "fe80::1"));
	ut_assertok(run_command("ping6 2001:db8:2::2", 0));
	ut_assertnull(netif_default);

	return 0;
}

static int ip6_test_dad(struct unit_test_state *uts, struct ip6_test *test)
{
	test->conflict = true;
	ut_asserteq(CMD_RET_FAILURE, run_command("ping6 fe80::2", 0));
	ut_asserteq(0, test->echoes);
	ut_assert(test->dad);
	ut_assertnull(netif_default);
	test->conflict = false;
	test->cancel = true;
	ut_asserteq(CMD_RET_FAILURE, run_command("ping6 fe80::2", 0));
	ut_asserteq(0, test->echoes);
	ut_assertnull(netif_default);
	ut_asserteq(0, test->bad_checksums);

	return 0;
}

static int ip6_test_dad_scope(struct unit_test_state *uts, struct ip6_test *test)
{
	ip_addr_t dest;
	ip6_addr_t alternate;

	ut_assertok(env_set("ip6addr", "2001:db8:1::10/64"));
	test->lease_conflict = true;
	ut_assertok(net_lwip_start(&test->net, NET_LWIP_ADDR_ENV_STRICT));
	ut_assertok(net_lwip_ip6_start(&test->net));
	ut_assert(ip6_addr_isduplicated(netif_ip6_addr_state(test->net.netif, 1)));
	ut_assertok(run_command("ping6 fe80::2", 0));
	ipaddr_aton("2001:db8:1::2", &dest);
	ut_asserteq(-EADDRINUSE, net_lwip_ip6_ready(&test->net, &dest));

	/* An independently configured address can still serve global traffic. */
	test->lease_conflict = false;
	ip6addr_aton("2001:db8:1::20", &alternate);
	netif_ip6_addr_set(test->net.netif, 2, &alternate);
	netif_ip6_addr_set_state(test->net.netif, 2, IP6_ADDR_TENTATIVE);
	ut_assertok(run_command("ping6 2001:db8:1::2", 0));
	ut_assert(ip6_addr_isvalid(netif_ip6_addr_state(test->net.netif, 2)));
	ipaddr_aton("2001:db8:2::2", &dest);
	ut_asserteq(-EAGAIN, net_lwip_ip6_ready(&test->net, &dest));
	ut_asserteq(2, test->echoes);
	ut_asserteq(0, test->bad_checksums);
	return 0;
}

#if CONFIG_IS_ENABLED(IPV6_ROUTER_DISCOVERY)
static void ip6_test_age(unsigned int seconds)
{
	timer_test_add_offset(seconds * 1000);
	net_lwip_ip6_update_lifetimes();
}

static int ip6_test_slaac(struct unit_test_state *uts, struct ip6_test *test)
{
	ip_addr_t dest;
	u32 before;

	test->advertise = true;
	ut_assertok(run_command("ping6 2001:db8:2::2", 0));
	ut_assert(test->solicitations);
	ut_assert(test->dad >= 2);
	ut_asserteq(1, test->echoes);
	ut_asserteq(0, test->bad_checksums);
	ut_assertnull(netif_default);
	/* Removal must discard the learned address, prefix and default router. */
	ut_assertok(net_lwip_start(&test->net, NET_LWIP_ADDR_ENV_FLEXIBLE));
	ut_assertok(net_lwip_ip6_init(&test->net));
	ut_assert(ip6_addr_isinvalid(netif_ip6_addr_state(test->net.netif, 1)));
	ipaddr_aton("2001:db8:2::2", &dest);
	ut_assertok(net_lwip_prepare_addr(&test->net, &dest));
	ip6_test_age(5);
	before = netif_ip6_addr_valid_life(test->net.netif, 1);
	ut_assertok(ip6_test_ra(test->net.dev));
	ut_assertok(net_lwip_poll());
	ut_assert(netif_ip6_addr_valid_life(test->net.netif, 1) > before);

	return 0;
}

static int ip6_test_lifetimes(struct unit_test_state *uts, struct ip6_test *test)
{
	ip_addr_t dest;
	u32 before;
	u8 state;
	unsigned int dad;

	ut_assertok(env_set("ipaddr", NULL));
	test->advertise = true;
	ut_assertok(net_lwip_start(&test->net, NET_LWIP_ADDR_ENV_STRICT));
	ipaddr_aton("2001:db8:2::2", &dest);
	ut_assertok(net_lwip_prepare_addr(&test->net, &dest));
	test->advertise = false;
	before = netif_ip6_addr_valid_life(test->net.netif, 1);
	/* Readiness must age retained state even before the next poll. */
	timer_test_add_offset(901000);
	ut_assertok(net_lwip_ip6_ready(&test->net, &dest));
	ut_asserteq(IP6_ADDR_DEPRECATED, netif_ip6_addr_state(test->net.netif, 1));
	ut_assert(netif_ip6_addr_valid_life(test->net.netif, 1) <= before - 901);
	timer_test_add_offset(900000);
	ut_assertok(net_lwip_poll());
	ut_asserteq(-EAGAIN, net_lwip_ip6_ready(&test->net, &dest));
	ut_assert(ip6_addr_isinvalid(netif_ip6_addr_state(test->net.netif, 1)));
	ut_assertnull(nd6_find_route(ip_2_ip6(&dest)));
	for (unsigned int i = 0; i < LWIP_ND6_NUM_PREFIXES; i++)
		ut_assert(prefix_list[i].netif != test->net.netif);

	/* A fresh advertisement is not aged by the preceding idle interval. */
	timer_test_add_offset(1801000);
	ut_assertok(ip6_test_ra_lifetimes(test->net.dev, U32_MAX, U32_MAX));
	ut_assertok(net_lwip_poll());
	ut_asserteq(IP6_ADDR_LIFE_INFINITE, netif_ip6_addr_valid_life(test->net.netif, 1));
	state = netif_ip6_addr_state(test->net.netif, 1);
	ut_assert(ip6_addr_istentative(state));
	dad = test->dad;
	ip6_test_age(600);
	/* Lifetime catch-up must neither send probes nor advance DAD. */
	ut_asserteq(dad, test->dad);
	ut_asserteq(state, netif_ip6_addr_state(test->net.netif, 1));
	ut_asserteq(IP6_ADDR_LIFE_INFINITE, netif_ip6_addr_valid_life(test->net.netif, 1));
	ut_asserteq(IP6_ADDR_LIFE_INFINITE, netif_ip6_addr_pref_life(test->net.netif, 1));
	ut_assertok(net_lwip_prepare_addr(&test->net, &dest));
	ip6_test_age(1801);
	ut_assert(ip6_addr_isvalid(netif_ip6_addr_state(test->net.netif, 1)));
	ut_assertnull(nd6_find_route(ip_2_ip6(&dest)));
	ipaddr_aton("2001:db8:1::2", &dest);
	ut_assertok(net_lwip_ip6_ready(&test->net, &dest));

	net_lwip_stop(&test->net);
	ut_assertok(env_set("ip6addr", "2001:db8:1::10"));
	ut_assertok(net_lwip_start(&test->net, NET_LWIP_ADDR_ENV_STRICT));
	ut_assertok(net_lwip_prepare_addr(&test->net, &dest));
	ip6_test_age(1801);
	ut_assertok(net_lwip_ip6_ready(&test->net, &dest));
	ut_assert(netif_ip6_addr_isstatic(test->net.netif, 1));
	ut_assert(ip6_addr_isvalid(netif_ip6_addr_state(test->net.netif, 0)));
	net_lwip_stop(&test->net);
	ut_assertok(env_set("ip6addr", NULL));
	timer_test_add_offset(1801000);
	ut_assertok(net_lwip_start(&test->net, NET_LWIP_ADDR_ENV_STRICT));
	ut_assertok(net_lwip_ip6_init(&test->net));
	ut_assert(ip6_addr_isinvalid(netif_ip6_addr_state(test->net.netif, 1)));
	return 0;
}
#endif

#if CONFIG_IS_ENABLED(CMD_TFTPBOOT)
static int ip6_test_tftp(struct unit_test_state *uts, struct ip6_test *test)
{
	void *data = map_sysmem(0x2100000, 9);
	ulong start;

	ut_assertok(env_set("ipaddr", NULL));
	ut_assertok(run_command("tftpboot 2100000 [fe80::2]:ipv6.bin", 0));
	ut_asserteq(9, net_boot_file_size);
	ut_asserteq_mem("ipv6-tftp", data, 9);
	ut_assertok(env_set("serverip6", "fe80::2"));
	test->fragment = true;
	ut_assertok(run_command("tftpboot 2100000 ipv6.bin -ipv6", 0));
	ut_asserteq(9, net_boot_file_size);
	ut_asserteq_mem("ipv6-tftp", data, 9);
	ut_asserteq(2, test->requests);
	ut_assert(test->fragments);
#if CONFIG_IS_ENABLED(CMD_TFTPPUT)
	ut_assertok(run_command("tftpput 2100000 9 [fe80::2]:ipv6.bin", 0));
	ut_asserteq(1, test->uploads);
#endif
	test->fragment = false;
	test->unreachable = true;
	start = get_timer(0);
	ut_asserteq(CMD_RET_FAILURE,
		    run_command("tftpboot 2100000 [fe80::2]:ipv6.bin", 0));
	ut_assert(get_timer(start) < 5000);
	ut_asserteq(0, test->bad_checksums);
	ut_assertnull(netif_default);
	unmap_sysmem(data);

	return 0;
}

static int ip6_test_tftp_numbers(struct unit_test_state *uts, struct ip6_test *test)
{
	static const char *const ports[] = {
		"65536", "4294967365", "18446744073709551685",
		"99999999999999999999999999999999",
	};
	static const char *const names[] = {
		"tftpdstp", "tftpsrcp", "tftpblocksize", "tftpwindowsize",
		"tftptimeout", "tftptimeoutcountmax",
	};
	static const char *const invalid[] = {
		"-1", "+69", " 69", "69junk", "18446744073709552640",
	};
	char port_arg[128];
	char *const port_argv[] = { "tftpboot", "2100000", port_arg };
	static char *const tftp_argv[] = {
		"tftpboot", "2100000", "ipv6.bin", "-ipv6",
	};
	int ret;

	ut_assertok(env_set("ipaddr", NULL));
	ut_assertok(env_set("serverip6", "fe80::2"));
	ut_assertok(net_lwip_start(&test->net, NET_LWIP_ADDR_ENV_FLEXIBLE));
	for (unsigned int i = 0; i < ARRAY_SIZE(ports); i++) {
		snprintf(port_arg, sizeof(port_arg), "[fe80::2]:%s:ipv6.bin",
			 ports[i]);
		ret = net_lwip_tftp_download(&test->net, ARRAY_SIZE(port_argv),
					     port_argv);
		ut_assert(ret);
		ut_asserteq(0, test->requests);
	}
	for (unsigned int i = 0; i < ARRAY_SIZE(names); i++) {
		if (i >= 2 && !CONFIG_IS_ENABLED(NET_TFTP_VARS))
			continue;
		for (unsigned int j = 0; j < ARRAY_SIZE(invalid); j++) {
			ut_assertok(env_set(names[i], invalid[j]));
			ret = net_lwip_tftp_download(&test->net,
						     ARRAY_SIZE(tftp_argv), tftp_argv);
			ut_assert(ret);
			ut_asserteq(0, test->requests);
		}
		ut_assertok(env_set(names[i], NULL));
	}
	/* Zero retains the default port; leading zeroes do not change the value. */
	ut_assertok(env_set("tftpsrcp", "0"));
	snprintf(port_arg, sizeof(port_arg), "[fe80::2]:0:ipv6.bin");
	ret = net_lwip_tftp_download(&test->net, ARRAY_SIZE(port_argv),
				     port_argv);
	ut_assertok(ret);
	snprintf(port_arg, sizeof(port_arg), "[fe80::2]:00065535:ipv6.bin");
	ret = net_lwip_tftp_download(&test->net, ARRAY_SIZE(port_argv),
				     port_argv);
	ut_assertok(ret);
	ut_asserteq(2, test->requests);
	net_lwip_stop(&test->net);
	ut_assertnull(netif_default);
	return 0;
}
#endif

static int ip6_test_run(struct unit_test_state *uts,
			int (*check)(struct unit_test_state *, struct ip6_test *))
{
	static const char * const names[] = {
		"ethact", "ethrotate", "netretry", "ip6addr", "gatewayip6",
		"ipaddr", "netmask", "serverip6", "filesize", "fileaddr",
		"dnsip", "dnsip2", "dnsip6", "dnsip6_2", "dnsresult",
		"autoload", "autostart", "bootfile", "pxe_configfile", "tftpdstp",
		"ncip", "ncip6", "ncinport", "ncoutport", "ntpserverip",
		"tftpsrcp", "tftpblocksize", "tftpwindowsize", "tftptimeout",
		"tftptimeoutcountmax",
	};
	struct ip6_test test = {};
#if CONFIG_IS_ENABLED(WGET)
	struct wget_http_info *http_info = wget_info;
#endif
	ulong load_addr = image_load_addr;
	u32 size = net_boot_file_size;
	char *saved[ARRAY_SIZE(names)];
	unsigned int i;
	int ret, ctrlc_state;

	for (i = 0; i < ARRAY_SIZE(names); i++) {
		const char *value = env_get(names[i]);

		saved[i] = value ? strdup(value) : NULL;
		if (value && !saved[i]) {
			while (i)
				free(saved[--i]);
			return -ENOMEM;
		}
	}
	env_set("ethact", "eth@10002000");
	env_set("ethrotate", "no");
	env_set("netretry", "no");
	env_set("ip6addr", NULL);
	env_set("gatewayip6", NULL);
	env_set("dnsip", NULL);
	env_set("dnsip2", NULL);
	env_set("dnsip6", NULL);
	env_set("dnsip6_2", NULL);
	env_set("tftpdstp", NULL);
	env_set("tftpsrcp", NULL);
	env_set("tftpblocksize", NULL);
	env_set("tftpwindowsize", NULL);
	env_set("tftptimeout", NULL);
	env_set("tftptimeoutcountmax", NULL);
	sandbox_eth_set_priv(0, &test);
	sandbox_eth_set_tx_handler(0, ip6_test_tx);
	ctrlc_state = disable_ctrlc(0);
	ret = check(uts, &test);
	disable_ctrlc(ctrlc_state);
	clear_ctrlc();
#if CONFIG_IS_ENABLED(WGET)
	wget_info = http_info;
#endif
	image_load_addr = load_addr;
	net_boot_file_size = size;
	net_lwip_stop(&test.net);
	sandbox_eth_set_tx_handler(0, NULL);
	sandbox_eth_set_priv(0, NULL);
	for (i = 0; i < ARRAY_SIZE(names); i++) {
		env_set(names[i], saved[i]);
		free(saved[i]);
	}
	return ret;
}

static int dm_test_lwip_ip6_config(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_config);
}
DM_TEST(dm_test_lwip_ip6_config, UTF_SCAN_FDT);

#if CONFIG_IS_ENABLED(WGET)
static int ip6_test_http_ports(struct unit_test_state *uts, struct ip6_test *test)
{
	static const char *const ports[] = {
		"", "0", "65536", "+8080", "8080junk", "4294975376",
		"18446744073709559696", "99999999999999999999999999999999",
	};
	struct wget_http_info info = { .method = WGET_HTTP_METHOD_GET };
	char url[128];

	for (unsigned int i = 0; i < ARRAY_SIZE(ports); i++) {
		snprintf(url, sizeof(url), "http://[fe80::2]:%s/ipv6.bin", ports[i]);
		ut_assert(wget_request(0x2100000, url, &info));
		ut_asserteq(0, test->http_requests);
		ut_assertnull(netif_default);
	}
	test->http_host = "Host: [fe80::2]:8080\r\n";
	ut_assertok(wget_request(0x2100000, "http://[fe80::2]:0008080/ipv6.bin", &info));
	ut_asserteq(1, test->http_requests);
	return 0;
}

static int dm_test_lwip_ip6_http_ports(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_http_ports);
}
DM_TEST(dm_test_lwip_ip6_http_ports, UTF_SCAN_FDT | UTF_CONSOLE);

static int ip6_test_http_scheme(struct unit_test_state *uts, struct ip6_test *test)
{
	static const char *const paths[] = {
		"/http://other.test/image", "/https://other.test/image",
		"/image?next=http://other.test/image",
		"/image?next=https://other.test/image",
	};
	struct wget_http_info info = { .method = WGET_HTTP_METHOD_GET };
	char url[128], target[128];

	test->http_host = "Host: [fe80::2]:8080\r\n";
	for (unsigned int i = 0; i < ARRAY_SIZE(paths); i++) {
		snprintf(url, sizeof(url), "http://[fe80::2]:8080%s", paths[i]);
		snprintf(target, sizeof(target), "GET %s HTTP/1.1\r\n", paths[i]);
		test->http_target = target;
		ut_assertok(wget_request(0x2100000, url, &info));
		ut_asserteq(i + 1, test->http_requests);
		ut_asserteq(9, info.file_size);
		ut_assertnull(netif_default);
	}
	return 0;
}

static int dm_test_lwip_ip6_http_scheme(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_http_scheme);
}
DM_TEST(dm_test_lwip_ip6_http_scheme, UTF_SCAN_FDT | UTF_CONSOLE);

#if CONFIG_IS_ENABLED(WGET_CACERT) || CONFIG_IS_ENABLED(WGET_BUILTIN_CACERT)
static int ip6_test_https_run(struct unit_test_state *uts,
			      int (*check)(struct unit_test_state *, struct ip6_test *))
{
	enum auth_mode saved_auth = cacert_auth_mode;
	size_t saved_size = cacert_size;
#if CONFIG_IS_ENABLED(WGET_BUILTIN_CACERT)
	bool saved_initialized = cacert_initialized;
#endif
	int ret;

	/* HTTPS must fail authentication rather than download over plain HTTP. */
	cacert_auth_mode = AUTH_REQUIRED;
	cacert_size = 0;
#if CONFIG_IS_ENABLED(WGET_BUILTIN_CACERT)
	cacert_initialized = true;
#endif
	ret = ip6_test_run(uts, check);
	cacert_auth_mode = saved_auth;
	cacert_size = saved_size;
#if CONFIG_IS_ENABLED(WGET_BUILTIN_CACERT)
	cacert_initialized = saved_initialized;
#endif
	return ret;
}

static int ip6_test_https_scheme(struct unit_test_state *uts, struct ip6_test *test)
{
	char urls[][128] = {
		"https://[fe80::3]:8443/ipv6.bin",
		"https://[fe80::3]:8443/http://[fe80::2]:8080/ipv6.bin",
		"https://[fe80::3]:8443/image?next=http://[fe80::2]:8080/ipv6.bin",
	};

	ut_assertok(env_set("ipaddr", NULL));
	test->http_host = "Host: [fe80::2]:8080\r\n";
	for (unsigned int i = 0; i < ARRAY_SIZE(urls); i++) {
		struct wget_http_info info = { .method = WGET_HTTP_METHOD_GET };
		int ret;

		ut_assert(wget_validate_uri(urls[i]));
		ret = wget_request(0x2100000, urls[i], &info);
		ut_asserteq(0, test->http_requests);
		ut_asserteq(-EINVAL, ret);
		ut_asserteq(0, info.file_size);
		ut_assertnull(netif_default);
	}
	return 0;
}

static int dm_test_lwip_ip6_https_scheme(struct unit_test_state *uts)
{
	return ip6_test_https_run(uts, ip6_test_https_scheme);
}
DM_TEST(dm_test_lwip_ip6_https_scheme, UTF_SCAN_FDT | UTF_CONSOLE);
#endif
#endif

static int ip6_test_prefix(struct unit_test_state *uts, struct ip6_test *test)
{
	ip6_addr_t addr, dest, gateway;
	char text[64];

	ip6addr_aton("2001:db8:1::10", &addr);
	for (unsigned int bits = 0; bits <= 128; bits++) {
		snprintf(text, sizeof(text), "2001:db8:1::10/%u", bits);
		ut_assertok(env_set("ip6addr", text));
		ut_assertok(net_lwip_start(&test->net, NET_LWIP_ADDR_ENV_FLEXIBLE));
		ut_assertok(net_lwip_ip6_init(&test->net));
		ut_assert(net_lwip_ip6_static_prefix(test->net.netif, 1, &addr));
		if (bits) {
			ip6_addr_copy(dest, addr);
			((u8 *)dest.addr)[(bits - 1) / 8] ^= BIT(7 - (bits - 1) % 8);
			ut_assert(!net_lwip_ip6_static_prefix(test->net.netif, 1, &dest));
		}
		if (bits < 128) {
			ip6_addr_copy(dest, addr);
			((u8 *)dest.addr)[bits / 8] ^= BIT(7 - bits % 8);
			ut_assert(net_lwip_ip6_static_prefix(test->net.netif, 1, &dest));
		}
		net_lwip_stop(&test->net);
	}
	ut_assertok(env_set("ip6addr", "2001:db8:1::10/48"));
	ut_assertok(run_command("ping6 2001:db8:1:ffff::2", 0));
	ip6addr_aton("2001:db8:1:ffff::2", &dest);
	ut_assert(ip6_addr_zoneless_eq(&dest, &test->last_neighbor));
	ut_assertok(env_set("ip6addr", "2001:db8:1::10/120"));
	ut_assertok(env_set("gatewayip6", "fe80::1"));
	ut_assertok(run_command("ping6 2001:db8:1::110", 0));
	ip6addr_aton("fe80::1", &gateway);
	ut_assert(ip6_addr_zoneless_eq(&gateway, &test->last_neighbor));
	ut_assertnull(netif_default);
	return 0;
}

static int dm_test_lwip_ip6_prefix(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_prefix);
}
DM_TEST(dm_test_lwip_ip6_prefix, UTF_SCAN_FDT);

#if CONFIG_IS_ENABLED(DNS)
static int ip6_test_dns(struct unit_test_state *uts, struct ip6_test *test)
{
	ulong start;

	ut_assertok(env_set("ipaddr", NULL));
	ut_assertok(env_set("dnsip6", "fe80::2"));
	ut_assertok(run_command("dns ipv6.test dnsresult -ipv6", 0));
	ut_asserteq_str("FE80::2", env_get("dnsresult"));
	ut_assertok(run_command("ping6 ping.test", 0));
	ut_asserteq(2, test->dns_queries);
	ut_asserteq(1, test->echoes);
	ut_assertnull(netif_default);

	/* An unavailable IPv6 backup must not delay a usable IPv4 primary. */
	ut_assertok(env_set("ipaddr", "192.0.2.10"));
	ut_assertok(env_set("netmask", "255.255.255.0"));
	ut_assertok(env_set("dnsip", "192.0.2.53"));
	ut_assertok(env_set("dnsip6", "2001:db8:2::53"));
	start = get_timer(0);
	ut_assertok(run_command("dns mixed.test dnsresult -ipv6", 0));
	ut_assert(get_timer(start) < 5000);
	ut_asserteq_str("FE80::2", env_get("dnsresult"));
	ut_asserteq(1, test->dns4_queries);
	ut_assertnull(netif_default);

	/* Also skip an unready primary in favor of a usable same-family backup. */
	ut_assertok(env_set("ipaddr", NULL));
	ut_assertok(env_set("dnsip", NULL));
	ut_assertok(env_set("dnsip6_2", "fe80::3"));
	ut_assertok(run_command("dns backup.test dnsresult -ipv6", 0));
	ut_asserteq_str("FE80::3", ip6addr_ntoa(&test->last_dns));
	return 0;
}

static int dm_test_lwip_ip6_dns(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_dns);
}
DM_TEST(dm_test_lwip_ip6_dns, UTF_SCAN_FDT);
#endif

static int ip6_test_apps(struct unit_test_state *uts, struct ip6_test *test)
{
	void *data = map_sysmem(0x2100000, 9);

	ut_assertok(env_set("ipaddr", NULL));
#if CONFIG_IS_ENABLED(WGET)
	{
		struct wget_http_info info = { .method = WGET_HTTP_METHOD_GET };

		test->http_host = "Host: [fe80::2]:8080\r\n";
		ut_assertok(wget_request(0x2100000, "http://[fe80::2]:8080/ipv6.bin", &info));
		ut_asserteq_mem("ipv6-http", data, 9);
		ut_asserteq(9, info.file_size);
		ut_asserteq(1, test->http_requests);
#if CONFIG_IS_ENABLED(DNS)
		ut_assertok(env_set("dnsip6", "fe80::2"));
		test->http_host = "Host: http.test:8080\r\n";
		ut_assertok(wget_request(0x2100000, "http://http.test:8080/ipv6.bin", &info));
		ut_asserteq_mem("ipv6-http", data, 9);
#endif
	}
#endif
#if CONFIG_IS_ENABLED(CMD_SNTP)
	ut_assertok(run_command("sntp fe80::2", 0));
	ut_asserteq(1, test->sntp_requests);
#endif
#if CONFIG_IS_ENABLED(CMD_NFS)
	ut_assertok(run_command("nfs 2100000 [fe80::2]:/ipv6.bin", 0));
	ut_asserteq_mem("ipv6-nfs!", data, 9);
	ut_asserteq(7, test->rpc_requests);
#endif
#if CONFIG_IS_ENABLED(NETCONSOLE)
	{
		struct stdio_dev *nc = stdio_get_by_name("nc");
		int ret;

		ut_assertnonnull(nc);
		ut_assertok(env_set("ncip", NULL));
		ut_assertok(env_set("ncip6", "fe80::2"));
		ut_assertok(env_set("ncinport", NULL));
		ut_assertok(env_set("ncoutport", NULL));
		ut_assertok(nc->start(nc));
		nc->puts(nc, "ipv6-console");
		ret = nc->tstc(nc);
		if (ret)
			ret = nc->getc(nc);
		nc->stop(nc);
		ut_asserteq('x', ret);
		ut_asserteq(1, test->console_packets);
	}
#endif
	ut_asserteq(0, test->bad_checksums);
	ut_assertnull(netif_default);
	unmap_sysmem(data);
	return 0;
}

static int dm_test_lwip_ip6_apps(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_apps);
}
DM_TEST(dm_test_lwip_ip6_apps, UTF_SCAN_FDT);

#if CONFIG_IS_ENABLED(NETCONSOLE)
static int ip6_test_netconsole_refresh(struct unit_test_state *uts, struct ip6_test *test)
{
	static const struct {
		const char *name;
		const char *value;
		const char *peer;
		int expected;
	} cases[] = {
		{ NULL, NULL, "fe80::2", 'x' },
		{ "filesize", "1234", "fe80::2", 'x' },
		{ "dnsip6", "fe80::3", "fe80::2", 'x' },
		{ "ncip6", "fe80::3", "fe80::2", 0 },
		{ NULL, NULL, "fe80::3", 'x' },
	};
	struct {
		struct udp_hdr udp;
		u8 data;
	} __packed packet = {
		.udp.src = htons(6666),
		.udp.dest = htons(6666),
		.udp.len = htons(sizeof(packet)),
		.data = 'x',
	};
	struct stdio_dev *nc = stdio_get_by_name("nc");
	int received[ARRAY_SIZE(cases)] = {};
	ip6_addr_t src;
	int ret = 0;

	ut_assertnonnull(nc);
	ut_assertok(env_set("ipaddr", NULL));
	ut_assertok(env_set("ncip", NULL));
	ut_assertok(env_set("ncip6", "fe80::2"));
	ut_assertok(env_set("ncinport", NULL));
	ut_assertok(env_set("ncoutport", NULL));
	ut_assertok(nc->start(nc));
	/* Input must retain the peer's scope without any console output. */
	for (unsigned int i = 0; i < ARRAY_SIZE(cases); i++) {
		if (cases[i].name) {
			ret = env_set(cases[i].name, cases[i].value);
			if (ret)
				break;
		}
		ip6addr_aton(cases[i].peer, &src);
		ret = ip6_test_reply(eth_get_dev(), &src, netif_ip6_addr(netif_default, 0),
				     &packet, sizeof(packet), IP6_NEXTH_UDP, false);
		if (ret)
			break;
		if (nc->tstc(nc))
			received[i] = nc->getc(nc);
	}
	nc->stop(nc);

	ut_assertok(ret);
	for (unsigned int i = 0; i < ARRAY_SIZE(cases); i++)
		ut_asserteq(cases[i].expected, received[i]);
	ut_asserteq(0, test->console_packets);
	ut_asserteq(0, test->bad_checksums);
	ut_assertnull(netif_default);
	return 0;
}

static int dm_test_lwip_ip6_netconsole_refresh(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_netconsole_refresh);
}
DM_TEST(dm_test_lwip_ip6_netconsole_refresh, UTF_SCAN_FDT);
#endif

#if CONFIG_IS_ENABLED(NETCONSOLE) && CONFIG_IS_ENABLED(IPV6_ROUTER_DISCOVERY)
static void ip6_test_console_output(void *arg)
{
	struct stdio_dev *nc = arg;

	nc->puts(nc, "output from poller");
}

static int ip6_test_netconsole_lifetimes(struct unit_test_state *uts, struct ip6_test *test)
{
	static const struct {
		const char *address;
		const char *gateway;
		const char *peer;
		bool infinite;
		bool survives;
	} cases[] = {
		{ NULL, NULL, "2001:db8:2::2", false, false },
		{ "2001:db8:1::10", NULL, "2001:db8:2::2", false, false },
		{ "2001:db8:1::10", "fe80::1", "2001:db8:2::2", false, true },
		{ NULL, NULL, "2001:db8:1::2", true, true },
	};
	struct stdio_dev *nc = stdio_get_by_name("nc");

	ut_assertnonnull(nc);
	ut_assertok(env_set("ipaddr", NULL));
	ut_assertok(env_set("ncip", NULL));
	ut_assertok(env_set("ncinport", NULL));
	ut_assertok(env_set("ncoutport", NULL));
	test->console_no_reply = true;
	for (unsigned int i = 0; i < ARRAY_SIZE(cases); i++) {
		unsigned int before, after, recovered, polled;
		ulong start, elapsed;
		bool valid;
		int ret = 0;

		ut_assertok(env_set("ip6addr", cases[i].address));
		ut_assertok(env_set("gatewayip6", cases[i].gateway));
		ut_assertok(env_set("ncip6", cases[i].peer));
		test->advertise = !cases[i].gateway;
		test->console_packets = 0;
		ut_assertok(nc->start(nc));
		test->advertise = false;
		if (cases[i].infinite)
			ret = ip6_test_ra_lifetimes(eth_get_dev(), U32_MAX, U32_MAX);
		/* Resolve the neighbor, then leave the output-only console idle. */
		nc->puts(nc, "before expiry");
		before = test->console_packets;
		timer_test_add_offset(1801000);
		start = get_timer(0);
		nc->puts(nc, "after expiry");
		nc->puts(nc, "still after expiry");
		elapsed = get_timer(start);
		after = test->console_packets;
		valid = ip6_addr_isvalid(netif_ip6_addr_state(netif_default, 1));

		/* Output must also process advertisements that restore readiness. */
		if (!cases[i].survives && !ret) {
			ret = ip6_test_ra(eth_get_dev());
			for (unsigned int j = 0; j < 5 && !ret; j++) {
				timer_test_add_offset(1000);
				nc->puts(nc, "restored route and address");
			}
		}
		recovered = test->console_packets;
		/* Console output from a poll callback must tolerate -EBUSY. */
		sys_timeout(1, ip6_test_console_output, nc);
		timer_test_add_offset(10);
		if (!ret)
			ret = net_lwip_poll();
		sys_untimeout(ip6_test_console_output, nc);
		polled = test->console_packets;
		nc->stop(nc);

		ut_assertok(ret);
		ut_asserteq(1, before);
		ut_asserteq(before + 2 * cases[i].survives, after);
		ut_assert(elapsed < 1000);
		ut_asserteq(!!cases[i].address || cases[i].infinite, valid);
		ut_assert(recovered > before);
		ut_asserteq(recovered + 1, polled);
		ut_assertnull(netif_default);
	}
	return 0;
}

static int dm_test_lwip_ip6_netconsole_lifetimes(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_netconsole_lifetimes);
}
DM_TEST(dm_test_lwip_ip6_netconsole_lifetimes, UTF_SCAN_FDT);
#endif

#if CONFIG_IS_ENABLED(CMD_TFTPSRV)
static void ip6_test_wrq(void *arg)
{
	struct ip6_test *test = arg;
	struct {
		struct udp_hdr udp;
		u8 data[18];
	} __packed packet = {};
	ip6_addr_t src;

	if (test->listener_done)
		return;
	sys_timeout(1000, ip6_test_wrq, test);
	if (!netif_default || !ip6_addr_isvalid(netif_ip6_addr_state(netif_default, 0)))
		return;
	packet.udp.src = htons(1234);
	packet.udp.dest = htons(69);
	packet.udp.len = htons(sizeof(packet));
	put_unaligned_be16(2, packet.data);
	memcpy(packet.data + 2, "ipv6.bin\0octet\0", 15);
	ip6addr_aton("fe80::2", &src);
	ip6_test_reply(netif_default->state, &src, netif_ip6_addr(netif_default, 0),
		       &packet, sizeof(packet), IP6_NEXTH_UDP, false);
}

static int ip6_test_server(struct unit_test_state *uts, struct ip6_test *test)
{
	void *data = map_sysmem(0x2100000, 9);
	int ret;

	ut_assertok(env_set("ipaddr", NULL));
	ut_assertok(env_set("autostart", "no"));
	test->listener = true;
	sys_timeout(1, ip6_test_wrq, test);
	ret = run_command("tftpsrv 2100000 -ipv6", 0);
	sys_untimeout(ip6_test_wrq, test);
	ut_assertok(ret);
	ut_assert(test->listener_done);
	ut_asserteq(9, net_boot_file_size);
	ut_asserteq_mem("ipv6-tftp", data, 9);
	ut_assertnull(netif_default);
	unmap_sysmem(data);
	return 0;
}

static int dm_test_lwip_ip6_server(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_server);
}
DM_TEST(dm_test_lwip_ip6_server, UTF_SCAN_FDT);
#endif

#if CONFIG_IS_ENABLED(IPV6_ROUTER_DISCOVERY) && CONFIG_IS_ENABLED(DNS)
static int ip6_test_rdnss(struct unit_test_state *uts, struct ip6_test *test)
{
	ip_addr_t addr;

	ut_assertok(env_set("ipaddr", NULL));
	test->advertise = true;
	test->rdnss_lifetime = 10;
	ut_assertok(net_lwip_start(&test->net, NET_LWIP_ADDR_ENV_STRICT));
	/* Complete DAD's simulated time jumps before counting DNS queries. */
	ipaddr_aton("2001:db8:2::2", &addr);
	ut_assertok(net_lwip_prepare_addr(&test->net, &addr));
	for (unsigned int i = 0; i < 31; i++)
		dns_tmr();
	ut_assertok(net_lwip_resolve(&test->net, "rdnss.test", &addr, IPADDR_TYPE_V6));
	ut_asserteq(1, test->dns_queries);
	ip6_test_age(11);
	ut_assert(ip_addr_isany(dns_getserver(0)));
	ut_assertok(env_set("dnsip6", "fe80::3"));
	ut_assertok(net_lwip_dns_init(&test->net));
	ut_assertok(ip6_test_ra(test->net.dev));
	ut_assertok(net_lwip_poll());
	ipaddr_aton("fe80::3", &addr);
	ut_assert(ip6_addr_zoneless_eq(ip_2_ip6(&addr), ip_2_ip6(dns_getserver(0))));
	net_lwip_stop(&test->net);
	ut_assert(ip_addr_isany(dns_getserver(0)));
	return 0;
}

static int dm_test_lwip_ip6_rdnss(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_rdnss);
}
DM_TEST(dm_test_lwip_ip6_rdnss, UTF_SCAN_FDT);

static int ip6_test_rdnss_advertise(struct udevice *dev, const char *first_addr,
				    u32 first, const char *second_addr, u32 second)
{
	struct {
		struct ra_header ra;
		struct rdnss_option dns[2];
	} reply = {};
	ip6_addr_t src, dst, addr;
	unsigned int count = second_addr ? 2 : 1;

	ip6addr_aton("fe80::1", &src);
	ip6addr_aton("ff02::1", &dst);
	reply.ra.type = ICMP6_TYPE_RA;
	reply.ra.router_lifetime = htons(1800);
	for (unsigned int i = 0; i < count; i++) {
		reply.dns[i].type = ND6_OPTION_TYPE_RDNSS;
		reply.dns[i].length = sizeof(reply.dns[i]) / 8;
		reply.dns[i].lifetime = htonl(i ? second : first);
		ip6addr_aton(i ? second_addr : first_addr, &addr);
		ip6_addr_copy_to_packed(reply.dns[i].rdnss_address[0], addr);
	}
	return ip6_test_reply(dev, &src, &dst, &reply,
			      sizeof(reply.ra) + count * sizeof(reply.dns[0]),
			      IP6_NEXTH_ICMP6, false);
}

static int ip6_test_rdnss_servers(struct udevice *dev, u32 first, u32 second,
				  unsigned int count)
{
	return ip6_test_rdnss_advertise(dev, "fe80::2", first,
				       count == 2 ? "fe80::3" : NULL, second);
}

static int ip6_test_rdnss_identity(struct unit_test_state *uts, struct ip6_test *test)
{
	ut_assertok(env_set("ipaddr", NULL));
	ut_assertok(net_lwip_start(&test->net, NET_LWIP_ADDR_ENV_STRICT));
	ut_assertok(net_lwip_ip6_start(&test->net));
	ut_asserteq(-ENOENT, net_lwip_dns_refresh(&test->net));
	ut_assertok(ip6_test_rdnss_servers(test->net.dev, 100, 100, 2));
	ut_assertok(net_lwip_poll());

	/* Refreshing the backup alone must not create a duplicate primary. */
	ut_assertok(ip6_test_rdnss_advertise(test->net.dev, "fe80::3", 1, NULL, 0));
	ut_assertok(net_lwip_poll());
	ut_asserteq_str("FE80::2", ipaddr_ntoa(dns_getserver(0)));
	ut_asserteq_str("FE80::3", ipaddr_ntoa(dns_getserver(1)));
	ip6_test_age(1);
	ut_asserteq_str("FE80::2", ipaddr_ntoa(dns_getserver(0)));
	ut_assert(ip_addr_isany(dns_getserver(1)));

	/* Reordered advertisements update the lifetimes by address. */
	ut_assertok(ip6_test_rdnss_servers(test->net.dev, 100, 100, 2));
	ut_assertok(net_lwip_poll());
	ut_assertok(ip6_test_rdnss_advertise(test->net.dev, "fe80::3", 2, "fe80::2", 1));
	ut_assertok(net_lwip_poll());
	ip6_test_age(1);
	ut_assert(ip_addr_isany(dns_getserver(0)));
	ut_asserteq_str("FE80::3", ipaddr_ntoa(dns_getserver(1)));
	ip6_test_age(1);
	ut_assert(ip_addr_isany(dns_getserver(1)));

	/* A new address replaces the entry that would expire first. */
	ut_assertok(ip6_test_rdnss_servers(test->net.dev, 100, 10, 2));
	ut_assertok(net_lwip_poll());
	ut_assertok(ip6_test_rdnss_advertise(test->net.dev, "fe80::4", 50, NULL, 0));
	ut_assertok(net_lwip_poll());
	ut_asserteq_str("FE80::2", ipaddr_ntoa(dns_getserver(0)));
	ut_asserteq_str("FE80::4", ipaddr_ntoa(dns_getserver(1)));
	ut_assertok(ip6_test_rdnss_advertise(test->net.dev, "fe80::4", 0, NULL, 0));
	ut_assertok(net_lwip_poll());
	ut_assert(ip_addr_isany(dns_getserver(1)));
	ut_asserteq_str("FE80::2", ipaddr_ntoa(dns_getserver(0)));
	return 0;
}

static int dm_test_lwip_ip6_rdnss_identity(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_rdnss_identity);
}
DM_TEST(dm_test_lwip_ip6_rdnss_identity, UTF_SCAN_FDT);

static int ip6_test_rdnss_idle(struct unit_test_state *uts, struct ip6_test *test)
{
	ip_addr_t addr;

	ut_assertok(env_set("ipaddr", NULL));
	ut_assertok(net_lwip_start(&test->net, NET_LWIP_ADDR_ENV_STRICT));
	ut_assertok(net_lwip_ip6_start(&test->net));
	ut_asserteq(-ENOENT, net_lwip_dns_refresh(&test->net));
	ut_assertok(ip6_test_rdnss_servers(test->net.dev, 5, 0, 1));
	ut_assertok(net_lwip_poll());
	ut_assert(!ip_addr_isany(dns_getserver(0)));
	timer_test_add_offset(6000);
	ut_assertok(net_lwip_poll());
	ut_assert(ip_addr_isany(dns_getserver(0)));

	/* Expire the primary during DNS setup, before it can send a query. */
	for (unsigned int i = 0; i < 31; i++)
		dns_tmr();
	ut_assertok(ip6_test_rdnss_servers(test->net.dev, 5, U32_MAX, 2));
	ut_assertok(net_lwip_poll());
	timer_test_add_offset(6000);
	ut_assertok(net_lwip_resolve(&test->net, "idle-rdnss.test", &addr, IPADDR_TYPE_V6));
	ut_assert(test->dns_queries);
	ut_asserteq_str("FE80::3", ip6addr_ntoa(&test->last_dns));
	ut_assert(ip_addr_isany(dns_getserver(0)));
	ip6_test_age(1801);
	ut_asserteq_str("FE80::3", ipaddr_ntoa(dns_getserver(1)));

	/* Refresh after an idle interval, then explicitly withdraw it. */
	timer_test_add_offset(6000);
	ut_assertok(ip6_test_rdnss_advertise(test->net.dev, "fe80::3", 5, NULL, 0));
	ut_assertok(net_lwip_poll());
	ut_asserteq_str("FE80::3", ipaddr_ntoa(dns_getserver(1)));
	ip6_test_age(4);
	ut_asserteq_str("FE80::3", ipaddr_ntoa(dns_getserver(1)));
	ut_assertok(ip6_test_rdnss_advertise(test->net.dev, "fe80::3", 0, NULL, 0));
	ut_assertok(net_lwip_poll());
	ut_assert(ip_addr_isany(dns_getserver(1)));
	ut_assertok(ip6_test_rdnss_servers(test->net.dev, U32_MAX, 0, 1));
	ut_assertok(net_lwip_poll());
	net_lwip_stop(&test->net);
	ut_assert(ip_addr_isany(dns_getserver(0)));
	ut_assert(ip_addr_isany(dns_getserver(1)));
	return 0;
}

static int dm_test_lwip_ip6_rdnss_idle(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_rdnss_idle);
}
DM_TEST(dm_test_lwip_ip6_rdnss_idle, UTF_SCAN_FDT);

static int ip6_test_rdnss_backup(struct unit_test_state *uts, struct ip6_test *test)
{
	ip_addr_t addr;

	ut_assertok(env_set("ipaddr", NULL));
	ut_assertok(net_lwip_start(&test->net, NET_LWIP_ADDR_ENV_STRICT));
	ut_assertok(net_lwip_ip6_start(&test->net));
	ut_asserteq(-ENOENT, net_lwip_dns_refresh(&test->net));
	for (unsigned int i = 0; i < 31; i++)
		dns_tmr();
	ut_assertok(ip6_test_rdnss_servers(test->net.dev, 1, 100, 2));
	ut_assertok(net_lwip_poll());
	ip6_test_age(1);
	ut_assert(ip_addr_isany(dns_getserver(0)));
	ut_assert(!ip_addr_isany(dns_getserver(1)));
	ut_assertok(net_lwip_poll());
	ut_assertok(net_lwip_resolve(&test->net, "rdnss-expiry.test", &addr, IPADDR_TYPE_V6));
	ut_asserteq_str("FE80::3", ip6addr_ntoa(&test->last_dns));
	ut_asserteq(1, test->dns_queries);
	ip6_test_age(100);
	ut_assert(ip_addr_isany(dns_getserver(1)));

	/* A zero-lifetime advertisement must leave the other server usable too. */
	ut_assertok(ip6_test_rdnss_servers(test->net.dev, 100, 100, 2));
	ut_assertok(net_lwip_poll());
	ut_assertok(ip6_test_rdnss_servers(test->net.dev, 0, 0, 1));
	ut_assertok(net_lwip_poll());
	ut_assert(ip_addr_isany(dns_getserver(0)));
	ut_assert(!ip_addr_isany(dns_getserver(1)));
	ut_assertok(net_lwip_resolve(&test->net, "rdnss-withdraw.test", &addr, IPADDR_TYPE_V6));
	ut_asserteq_str("FE80::3", ip6addr_ntoa(&test->last_dns));
	ut_asserteq(2, test->dns_queries);
	net_lwip_stop(&test->net);
	ut_assert(ip_addr_isany(dns_getserver(0)));
	ut_assert(ip_addr_isany(dns_getserver(1)));
	return 0;
}

static int dm_test_lwip_ip6_rdnss_backup(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_rdnss_backup);
}
DM_TEST(dm_test_lwip_ip6_rdnss_backup, UTF_SCAN_FDT);

static int ip6_test_rdnss_override(struct unit_test_state *uts, struct ip6_test *test)
{
	static const char *const servers[] = { "fe80::2", "fe80::9" };
	ip_addr_t expected;

	ut_assertok(env_set("ipaddr", NULL));
	ut_assertok(net_lwip_start(&test->net, NET_LWIP_ADDR_ENV_STRICT));
	ut_assertok(net_lwip_ip6_start(&test->net));
	ut_asserteq(-ENOENT, net_lwip_dns_refresh(&test->net));
	ut_assertok(ip6_test_rdnss_servers(test->net.dev, 5, 0, 1));
	ut_assertok(net_lwip_poll());
	ut_assert(!ip_addr_isany(dns_getserver(0)));

	/* An override not yet installed must not cancel the learned lifetime. */
	ut_assertok(env_set("dnsip6", "fe80::9"));
	ip6_test_age(1);
	ut_assertok(env_set("dnsip6", NULL));
	ip6_test_age(5);
	ut_assert(ip_addr_isany(dns_getserver(0)));

	for (unsigned int i = 0; i < ARRAY_SIZE(servers); i++) {
		ut_assertok(ip6_test_rdnss_servers(test->net.dev, 5, 0, 1));
		ut_assertok(net_lwip_poll());
		ut_assertok(env_set("dnsip6", servers[i]));
		ut_assertok(net_lwip_dns_refresh(&test->net));
		ipaddr_aton(servers[i], &expected);
		ip6_addr_assign_zone(ip_2_ip6(&expected), IP6_UNKNOWN, test->net.netif);

		/* Explicit settings survive the old lifetime, even at the same address. */
		ip6_test_age(6);
		ut_assert(ip_addr_cmp(&expected, dns_getserver(0)));
		ut_assertok(env_set("dnsip6", NULL));
		ut_asserteq(-ENOENT, net_lwip_dns_refresh(&test->net));
		ut_assert(ip_addr_isany(dns_getserver(0)));

		/* RDNSS may arrive before the first refresh after removing an override. */
		ut_assertok(env_set("dnsip6", servers[i]));
		ut_assertok(net_lwip_dns_refresh(&test->net));
		ut_assertok(env_set("dnsip6", NULL));
		ut_assertok(ip6_test_rdnss_servers(test->net.dev, 5, 0, 1));
		ut_assertok(net_lwip_poll());
		ut_asserteq(-ENOENT, net_lwip_dns_refresh(&test->net));
		ut_asserteq_str("FE80::2", ipaddr_ntoa(dns_getserver(0)));
		ut_assert(ip_addr_isany(dns_getserver(1)));
		ip6_test_age(6);
		ut_assert(ip_addr_isany(dns_getserver(0)));
	}
	return 0;
}

static int dm_test_lwip_ip6_rdnss_override(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_rdnss_override);
}
DM_TEST(dm_test_lwip_ip6_rdnss_override, UTF_SCAN_FDT);

static int ip6_test_rdnss_chain(struct unit_test_state *uts, struct ip6_test *test)
{
	static const u8 invalid_lengths[] = { 1, 4, 7 };
	struct {
		struct ra_header ra;
		struct rdnss_option dns;
		ip6_addr_p_t extra;
	} reply = {};
	struct eth_sandbox_priv *priv;
	struct pbuf *head, *tail;
	ip6_addr_t src, dst, server;
	unsigned int first = ETHER_HDR_SIZE + IP6_HLEN + sizeof(reply.ra);
	unsigned int len;

	ut_assertok(env_set("ipaddr", NULL));
	ut_assertok(net_lwip_start(&test->net, NET_LWIP_ADDR_ENV_STRICT));
	ut_assertok(net_lwip_ip6_start(&test->net));
	ut_asserteq(-ENOENT, net_lwip_dns_refresh(&test->net));
	ip6addr_aton("fe80::1", &src);
	ip6addr_aton("ff02::1", &dst);
	reply.ra.type = ICMP6_TYPE_RA;
	reply.ra.router_lifetime = htons(1800);
	reply.dns.type = ND6_OPTION_TYPE_RDNSS;
	reply.dns.length = (sizeof(reply.dns) + sizeof(reply.extra)) / 8;
	reply.dns.lifetime = htonl(100);
	ip6addr_aton("fe80::2", &server);
	ip6_addr_copy_to_packed(reply.dns.rdnss_address[0], server);
	ip6addr_aton("fe80::3", &server);
	ip6_addr_copy_to_packed(reply.extra, server);
	ut_assertok(ip6_test_reply(test->net.dev, &src, &dst, &reply, sizeof(reply),
				 IP6_NEXTH_ICMP6, false));
	ut_assertok(net_lwip_poll());
	ut_assert(!ip_addr_isany(dns_getserver(0)));
	ut_assert(!ip_addr_isany(dns_getserver(1)));
	dns_setserver(0, NULL);
	dns_setserver(1, NULL);

	/* Deliver identical packet bytes with the RDNSS option in a second pbuf. */
	ut_assertok(ip6_test_reply(test->net.dev, &src, &dst, &reply, sizeof(reply),
				 IP6_NEXTH_ICMP6, false));
	priv = dev_get_priv(test->net.dev);
	ut_asserteq(1, priv->recv_packets);
	len = priv->recv_packet_length[0];
	head = pbuf_alloc(PBUF_RAW, first, PBUF_RAM);
	ut_assertnonnull(head);
	tail = pbuf_alloc(PBUF_RAW, len - first, PBUF_RAM);
	if (!tail)
		pbuf_free(head);
	ut_assertnonnull(tail);
	ut_asserteq(ERR_OK, pbuf_take(head, priv->recv_packet_buffer[0], first));
	ut_asserteq(ERR_OK, pbuf_take(tail, priv->recv_packet_buffer[0] + first, len - first));
	pbuf_cat(head, tail);
	priv->recv_packets = 0;
	ut_asserteq(ERR_OK, test->net.netif->input(head, test->net.netif));
	ut_asserteq_str("FE80::2", ipaddr_ntoa(dns_getserver(0)));
	ut_asserteq_str("FE80::3", ipaddr_ntoa(dns_getserver(1)));

	for (unsigned int i = 0; i < ARRAY_SIZE(invalid_lengths); i++) {
		dns_setserver(0, NULL);
		dns_setserver(1, NULL);
		reply.dns.length = invalid_lengths[i];
		ut_assertok(ip6_test_reply(test->net.dev, &src, &dst, &reply, sizeof(reply),
					 IP6_NEXTH_ICMP6, false));
		ut_assertok(net_lwip_poll());
		ut_assert(ip_addr_isany(dns_getserver(0)));
		ut_assert(ip_addr_isany(dns_getserver(1)));
	}
	return 0;
}

static int dm_test_lwip_ip6_rdnss_chain(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_rdnss_chain);
}
DM_TEST(dm_test_lwip_ip6_rdnss_chain, UTF_SCAN_FDT);
#endif

static int dm_test_lwip_ip6_ping(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_ping);
}
DM_TEST(dm_test_lwip_ip6_ping, UTF_SCAN_FDT);

static int dm_test_lwip_ip6_dad(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_dad);
}
DM_TEST(dm_test_lwip_ip6_dad, UTF_SCAN_FDT | UTF_CONSOLE);

static int dm_test_lwip_ip6_dad_scope(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_dad_scope);
}
DM_TEST(dm_test_lwip_ip6_dad_scope, UTF_SCAN_FDT);

#if CONFIG_IS_ENABLED(CMD_TFTPBOOT)
static int dm_test_lwip_ip6_tftp(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_tftp);
}
DM_TEST(dm_test_lwip_ip6_tftp, UTF_SCAN_FDT);

static int dm_test_lwip_ip6_tftp_numbers(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_tftp_numbers);
}
DM_TEST(dm_test_lwip_ip6_tftp_numbers, UTF_SCAN_FDT | UTF_CONSOLE);
#endif

#if CONFIG_IS_ENABLED(IPV6_ROUTER_DISCOVERY)
static int dm_test_lwip_ip6_slaac(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_slaac);
}
DM_TEST(dm_test_lwip_ip6_slaac, UTF_SCAN_FDT);

static int dm_test_lwip_ip6_lifetimes(struct unit_test_state *uts)
{
	return ip6_test_run(uts, ip6_test_lifetimes);
}
DM_TEST(dm_test_lwip_ip6_lifetimes, UTF_SCAN_FDT);
#endif
