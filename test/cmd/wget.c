// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022 Linaro
 *
 * (C) Copyright 2022
 * Ying-Chun Liu (PaulLiu) <paul.liu@linaro.org>
 */

#include <command.h>
#include <dm.h>
#include <env.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <net/tcp.h>
#include <net/wget.h>
#include <asm/eth.h>
#include <dm/test.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <test/cmd.h>
#include <test/test.h>
#include <test/ut.h>

#define SHIFT_TO_TCPHDRLEN_FIELD(x) ((x) << 4)
#define LEN_B_TO_DW(x) ((x) >> 2)
#define GET_TCP_HDR_LEN_IN_BYTES(x) ((x) >> 2)

static int sb_arp_handler(struct udevice *dev, void *packet,
			  unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct arp_hdr *arp = packet + ETHER_HDR_SIZE;
	int ret = 0;

	if (ntohs(arp->ar_op) == ARPOP_REQUEST) {
		priv->fake_host_ipaddr = net_read_ip(&arp->ar_spa);

		ret = sandbox_eth_recv_arp_req(dev);
		if (ret)
			return ret;
		ret = sandbox_eth_arp_req_to_reply(dev, packet, len);
		return ret;
	}

	return -EPROTONOSUPPORT;
}

static int sb_syn_handler(struct udevice *dev, void *packet,
			  unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth = packet;
	struct ip_tcp_hdr *tcp = packet + ETHER_HDR_SIZE;
	struct ethernet_hdr *eth_send;
	struct ip_tcp_hdr *tcp_send;

	/* Don't allow the buffer to overrun */
	if (priv->recv_packets >= PKTBUFSRX)
		return 0;

	eth_send = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(eth_send->et_dest, eth->et_src, ARP_HLEN);
	memcpy(eth_send->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	priv->irs = ntohl(tcp->tcp_seq);
	priv->iss = ~priv->irs; /* just to differ from irs */
	eth_send->et_protlen = htons(PROT_IP);
	tcp_send = (void *)eth_send + ETHER_HDR_SIZE;
	tcp_send->tcp_src = tcp->tcp_dst;
	tcp_send->tcp_dst = tcp->tcp_src;
	tcp_send->tcp_seq = htonl(priv->iss);
	tcp_send->tcp_ack = htonl(priv->irs + 1);
	tcp_send->tcp_hlen = SHIFT_TO_TCPHDRLEN_FIELD(LEN_B_TO_DW(TCP_HDR_SIZE));
	tcp_send->tcp_flags = TCP_SYN | TCP_ACK;
	tcp_send->tcp_win = htons(PKTBUFSRX * TCP_MSS >> TCP_SCALE);
	tcp_send->tcp_xsum = 0;
	tcp_send->tcp_ugr = 0;
	tcp_send->tcp_xsum = tcp_set_pseudo_header((uchar *)tcp_send,
						   tcp->ip_src,
						   tcp->ip_dst,
						   TCP_HDR_SIZE,
						   IP_TCP_HDR_SIZE);
	net_set_ip_header((uchar *)tcp_send,
			  tcp->ip_src,
			  tcp->ip_dst,
			  IP_TCP_HDR_SIZE,
			  IPPROTO_TCP);

	priv->recv_packet_length[priv->recv_packets] =
		ETHER_HDR_SIZE + IP_TCP_HDR_SIZE;
	++priv->recv_packets;

	return 0;
}

static int sb_ack_handler(struct udevice *dev, void *packet,
			  unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct ethernet_hdr *eth = packet;
	struct ip_tcp_hdr *tcp = packet + ETHER_HDR_SIZE;
	struct ethernet_hdr *eth_send;
	struct ip_tcp_hdr *tcp_send;
	void *data;
	int pkt_len;
	int payload_len = 0;
	u32 tcp_seq, tcp_ack;
	int tcp_data_len;
	const char *payload1 =
		/* response status line */
		"HTTP/1.1 200 OK\r\n"
		/* response header fields */
		"Date: Mon, 23 Dec 2024 05:18:23 GMT\r\n"
		"Server: Apache/2.4.62 (Debian)\r\n"
		"Last-Modified: Mon, 23 Dec 2024 05:04:50 GMT\r\n"
		"ETag: \"1d-629e8efb09e7b\"\r\n"
		"Accept-Ranges: bytes\r\n"
		"Content-Length: 29\r\n"
		"Connection: close\r\n"
		"Content-Type: text/html\r\n"
		/* response header fields end marker */
		"\r\n"
		/* file data (for HTTP GET requests) */
		"<html><body>Hi</body></html>\n";

	/* Don't allow the buffer to overrun */
	if (priv->recv_packets >= PKTBUFSRX)
		return 0;

	eth_send = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(eth_send->et_dest, eth->et_src, ARP_HLEN);
	memcpy(eth_send->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	eth_send->et_protlen = htons(PROT_IP);
	tcp_send = (void *)eth_send + ETHER_HDR_SIZE;
	tcp_send->tcp_src = tcp->tcp_dst;
	tcp_send->tcp_dst = tcp->tcp_src;
	data = (void *)tcp_send + IP_TCP_HDR_SIZE;

	tcp_seq = ntohl(tcp->tcp_seq) - priv->irs;
	tcp_ack = ntohl(tcp->tcp_ack) - priv->iss;
	tcp_data_len = len - ETHER_HDR_SIZE - IP_HDR_SIZE - GET_TCP_HDR_LEN_IN_BYTES(tcp->tcp_hlen);

	if (tcp->tcp_flags & TCP_FIN)
		tcp_data_len++;

	tcp_send->tcp_seq = htonl(ntohl(tcp->tcp_ack));
	tcp_send->tcp_ack = htonl(ntohl(tcp->tcp_seq) + tcp_data_len);

	if (tcp_seq == 1 && tcp_ack == 1) {
		if (tcp_data_len == 0) {
			/* no data, wait for GET/HEAD request */
			return -1;
		}

		/* reply to GET/HEAD request */
		payload_len = strlen(payload1);
		memcpy(data, payload1, payload_len);
		tcp_send->tcp_flags = TCP_ACK;
	} else if (tcp_ack == 1 + strlen(payload1)) {
		payload_len = 0;
		tcp_send->tcp_flags = TCP_ACK | TCP_FIN;
	} else if (tcp_ack == 2 + strlen(payload1)) {
		payload_len = 0;
		tcp_send->tcp_flags = TCP_ACK;
	}

	tcp_send->tcp_hlen = SHIFT_TO_TCPHDRLEN_FIELD(LEN_B_TO_DW(TCP_HDR_SIZE));
	tcp_send->tcp_win = htons(PKTBUFSRX * TCP_MSS >> TCP_SCALE);
	tcp_send->tcp_xsum = 0;
	tcp_send->tcp_ugr = 0;
	pkt_len = IP_TCP_HDR_SIZE + payload_len;
	tcp_send->tcp_xsum = tcp_set_pseudo_header((uchar *)tcp_send,
						   tcp->ip_src,
						   tcp->ip_dst,
						   pkt_len - IP_HDR_SIZE,
						   pkt_len);
	net_set_ip_header((uchar *)tcp_send,
			  tcp->ip_src,
			  tcp->ip_dst,
			  pkt_len,
			  IPPROTO_TCP);

	priv->recv_packet_length[priv->recv_packets] =
		ETHER_HDR_SIZE + IP_TCP_HDR_SIZE + payload_len;
	++priv->recv_packets;

	return 0;
}

static int sb_http_handler(struct udevice *dev, void *packet,
			   unsigned int len)
{
	struct ethernet_hdr *eth = packet;
	struct ip_hdr *ip;
	struct ip_tcp_hdr *tcp;

	if (ntohs(eth->et_protlen) == PROT_ARP) {
		return sb_arp_handler(dev, packet, len);
	} else if (ntohs(eth->et_protlen) == PROT_IP) {
		ip = packet + ETHER_HDR_SIZE;
		if (ip->ip_p == IPPROTO_TCP) {
			tcp = packet + ETHER_HDR_SIZE;
			if (tcp->tcp_flags == TCP_SYN)
				return sb_syn_handler(dev, packet, len);
			else if (tcp->tcp_flags & TCP_ACK && !(tcp->tcp_flags & TCP_SYN))
				return sb_ack_handler(dev, packet, len);
			return 0;
		}
		return -EPROTONOSUPPORT;
	}

	return -EPROTONOSUPPORT;
}

static int net_test_wget(struct unit_test_state *uts)
{
	char *prev_ethact = env_get("ethact");
	char *prev_ethrotate = env_get("ethrotate");

	sandbox_eth_set_tx_handler(0, sb_http_handler);
	sandbox_eth_set_priv(0, uts);

	env_set("ethact", "eth@10002000");
	env_set("ethrotate", "no");
	env_set("wgetaddr", "0x20000");
	ut_assertok(run_command("wget ${wgetaddr} 1.1.2.2:/index.html", 0));
	ut_assert_nextline_empty();
	ut_assert_nextline("Packets received 5, Transfer Successful");
	ut_assert_nextline("Bytes transferred = 29 (1d hex)");

	sandbox_eth_set_tx_handler(0, NULL);

	run_command("md5sum ${wgetaddr} ${filesize}", 0);
	ut_assert_nextline("md5 for 00020000 ... 0002001c ==> 847d5e7320a27462e90bc1ed75eb8cd8");
	ut_assert_console_end();

	env_set("ethact", prev_ethact);
	env_set("ethrotate", prev_ethrotate);

	return 0;
}
CMD_TEST(net_test_wget, UTF_CONSOLE);

static int net_test_wget_uri_validate(struct unit_test_state *uts)
{
	ut_asserteq(true, wget_validate_uri("http://foo.com/bar.html"));
	ut_asserteq(true, wget_validate_uri("http://1.1.2.3/bar.html"));
	ut_asserteq(false, wget_validate_uri("http://foo/ba r.html"));
	ut_asserteq(false, wget_validate_uri("http://"));

	if (CONFIG_IS_ENABLED(WGET_HTTPS)) {
		ut_asserteq(true,
			    wget_validate_uri("https://foo.com/bar.html"));
		ut_asserteq(true,
			    wget_validate_uri("https://1.1.2.3/bar.html"));
		ut_asserteq(false, wget_validate_uri("https://foo/ba r.html"));
		ut_asserteq(false, wget_validate_uri("https://"));
	}

	return 0;
}
CMD_TEST(net_test_wget_uri_validate, UTF_CONSOLE);
