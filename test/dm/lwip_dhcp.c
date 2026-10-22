// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2026 James Hilliard <james.hilliard1@gmail.com> */

#include <command.h>
#include <dm.h>
#include <env.h>
#include <image.h>
#include <malloc.h>
#include <net.h>
#include <time.h>
#include <asm/eth.h>
#include <asm/unaligned.h>
#include <dm/test.h>
#include <lwip/dhcp.h>
#include <lwip/prot/dhcp.h>
#include <lwip/prot/etharp.h>
#include <test/ut.h>

struct dhcp_handoff_test {
	const char *offer;
	bool fail;
	bool timeout;
	bool dhcp_alive;
	bool rx_burst;
	unsigned int rx_packets;
	int rx_error;
	unsigned int arp_replies;
	unsigned int tftp;
	unsigned int nfs;
	u32 boot_server;
	u32 nfs_server;
	char filename[128];
};

static int dhcp_handoff_reply(struct udevice *dev, struct dhcp_msg *request)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct dhcp_handoff_test *test = priv->priv;
	struct ethernet_hdr *eth;
	struct ip_udp_hdr *ip;
	struct dhcp_msg *reply;
	u8 type = request->options[2];
	u8 *pos;
	size_t len;

	if (type != DHCP_DISCOVER && type != DHCP_REQUEST)
		return 0;
	if (test->timeout) {
		timer_test_add_offset(10001);
		return 0;
	}
	if (priv->recv_packets >= PKTBUFSRX)
		return -ENOSPC;
	eth = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memset(eth->et_dest, 0xff, ARP_HLEN);
	memcpy(eth->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	eth->et_protlen = htons(PROT_IP);
	ip = (void *)(eth + 1);
	memset(ip, 0, IP_UDP_HDR_SIZE);
	reply = (void *)(ip + 1);
	memset(reply, 0, sizeof(*reply));
	reply->op = DHCP_BOOTREPLY;
	reply->htype = 1;
	reply->hlen = ARP_HLEN;
	reply->xid = request->xid;
	memcpy(reply->chaddr, request->chaddr, sizeof(reply->chaddr));
	put_unaligned_be32(0x01010203, &reply->yiaddr);
	put_unaligned_be32(test->boot_server, &reply->siaddr);
	reply->cookie = htonl(DHCP_MAGIC_COOKIE);
	strlcpy((char *)reply->file, test->offer, sizeof(reply->file));
	pos = reply->options;
	*pos++ = DHCP_OPTION_MESSAGE_TYPE;
	*pos++ = 1;
	*pos++ = type == DHCP_DISCOVER ? DHCP_OFFER : DHCP_ACK;
	*pos++ = DHCP_OPTION_SERVER_ID;
	*pos++ = 4;
	put_unaligned_be32(0x01010202, pos);
	pos += 4;
	*pos++ = DHCP_OPTION_SUBNET_MASK;
	*pos++ = 4;
	put_unaligned_be32(0xffffff00, pos);
	pos += 4;
	*pos++ = DHCP_OPTION_LEASE_TIME;
	*pos++ = 4;
	put_unaligned_be32(3600, pos);
	pos += 4;
	*pos++ = DHCP_OPTION_END;
	len = pos - (u8 *)reply;
	ip->ip_hl_v = 0x45;
	ip->ip_len = htons(IP_UDP_HDR_SIZE + len);
	ip->ip_ttl = 64;
	ip->ip_p = IPPROTO_UDP;
	ip->ip_src.s_addr = htonl(0x01010202);
	ip->ip_dst.s_addr = htonl(0xffffffff);
	ip->udp_src = htons(67);
	ip->udp_dst = htons(68);
	ip->udp_len = htons(UDP_HDR_SIZE + len);
	ip->ip_sum = compute_ip_checksum(ip, IP_HDR_SIZE);
	priv->recv_packet_length[priv->recv_packets++] = ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + len;

	return 0;
}

static int dhcp_handoff_tx(struct udevice *dev, void *packet, unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct dhcp_handoff_test *test = priv->priv;
	struct ethernet_hdr *eth = packet, *reply_eth;
	struct ip_udp_hdr *ip, *reply_ip;
	u8 *data;

	if (test->rx_burst && netif_dhcp_data(netif_default) &&
	    ntohs(eth->et_protlen) == PROT_ARP) {
		struct etharp_hdr *arp = (void *)(eth + 1);

		if (ntohs(arp->opcode) == ARP_REPLY)
			test->arp_replies++;
		return 0;
	}
	if (!sandbox_eth_arp_req_to_reply(dev, packet, len))
		return 0;
	if (len < ETHER_HDR_SIZE + IP_UDP_HDR_SIZE ||
	    ntohs(eth->et_protlen) != PROT_IP)
		return 0;
	ip = (void *)(eth + 1);
	if (ip->ip_p != IPPROTO_UDP)
		return 0;
	if (ntohs(ip->udp_dst) == 67) {
		if (len < ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + DHCP_OPTIONS_OFS + 3)
			return -EINVAL;
		return dhcp_handoff_reply(dev, (void *)(ip + 1));
	}
	if (ntohs(ip->udp_dst) == 111) {
		test->nfs++;
		test->nfs_server = ntohl(ip->ip_dst.s_addr);
		test->dhcp_alive |= !!netif_dhcp_data(netif_default);
		/* Terminate the RPC request: downloading NFS data is not this test. */
		net_set_state(NETLOOP_FAIL);
		return 0;
	}
	if (ntohs(ip->udp_dst) != 69 ||
	    len < ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + 4 ||
	    get_unaligned_be16(ip + 1) != 1)
		return 0;
	test->tftp++;
	test->dhcp_alive |= !!netif_dhcp_data(netif_default);
	strlcpy(test->filename, (char *)(ip + 1) + 2, sizeof(test->filename));
	if (priv->recv_packets >= PKTBUFSRX)
		return -ENOSPC;
	reply_eth = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(reply_eth->et_dest, eth->et_src, ARP_HLEN);
	memcpy(reply_eth->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	reply_eth->et_protlen = htons(PROT_IP);
	reply_ip = (void *)(reply_eth + 1);
	memset(reply_ip, 0, IP_UDP_HDR_SIZE);
	reply_ip->ip_hl_v = 0x45;
	reply_ip->ip_len = htons(IP_UDP_HDR_SIZE + 6);
	reply_ip->ip_ttl = 64;
	reply_ip->ip_p = IPPROTO_UDP;
	reply_ip->ip_src = ip->ip_dst;
	reply_ip->ip_dst = ip->ip_src;
	reply_ip->udp_src = htons(1234);
	reply_ip->udp_dst = ip->udp_src;
	reply_ip->udp_len = htons(UDP_HDR_SIZE + 6);
	reply_ip->ip_sum = compute_ip_checksum(reply_ip, IP_HDR_SIZE);
	data = (void *)(reply_ip + 1);
	put_unaligned_be16(test->fail ? 5 : 3, data);
	put_unaligned_be16(1, data + 2);
	data[4] = 'x';
	data[5] = 0;
	priv->recv_packet_length[priv->recv_packets++] =
		ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + 6;

	return 0;
}

static void dhcp_handoff_free_pkt(struct udevice *dev, int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct dhcp_handoff_test *test = priv->priv;
	struct etharp_hdr *arp;
	int ret;

	if (!test->rx_burst || len <= 0)
		return;
	test->rx_packets++;
	if (test->rx_packets >= ETH_PACKETS_BATCH_RECV ||
	    !dhcp_supplied_address(netif_default))
		return;
	priv->fake_host_ipaddr.s_addr = htonl(0x01010202);
	ret = sandbox_eth_recv_arp_req(dev);
	if (ret) {
		test->rx_error = ret;
		return;
	}
	arp = (void *)(priv->recv_packet_buffer[priv->recv_packets - 1] +
		       ETHER_HDR_SIZE);
	memcpy(&arp->dipaddr, netif_ip4_addr(netif_default),
	       sizeof(arp->dipaddr));
}

static int dhcp_handoff_check(struct unit_test_state *uts,
			      struct dhcp_handoff_test *test)
{
	const char * const autoload[] = { NULL, "yes-please", "no", "NFS" };
	char * const argv[] = { "dhcp", "2000000", "explicit.bin" };
	int i, ret;

	ut_assertok(env_set("ethact", "eth@10002000"));
	ut_assertok(env_set("serverip", "1.1.2.2"));
	ut_assertok(env_set("loadaddr", "2000000"));
	ut_assertok(env_set("autostart", "no"));
	ut_assertok(env_set("nfsserverip", NULL));
	ut_assertok(env_set("tftpdstp", NULL));
	ut_assertok(env_set("tftpsrcp", NULL));
	for (i = 0; i < 4; i++) {
		*test = (struct dhcp_handoff_test){ .offer = "offered.bin" };
		ut_assertok(env_set("autoload", i == 3 ? "no" : "yes"));
		ut_assertok(env_set("bootfile", "default.bin"));
		ut_assertok(env_set("tftpserverip", "9.9.9.9"));
		if (i == 2)
			test->offer = "";
		ret = do_dhcp(NULL, 0, i == 1 || i == 3 ? 3 : 1, argv);
		ut_asserteq(i != 3 && !IS_ENABLED(CONFIG_CMD_TFTPBOOT) ?
			   CMD_RET_FAILURE : CMD_RET_SUCCESS, ret);
		ut_asserteq(i != 3 && IS_ENABLED(CONFIG_CMD_TFTPBOOT), test->tftp);
		ut_assert(!test->dhcp_alive);
		ut_assertnull(env_get("tftpserverip"));
		if (test->tftp)
			ut_asserteq_str(i == 1 ? "explicit.bin" : i == 2 ?
					"default.bin" : "offered.bin", test->filename);
		ut_assertnull(netif_default);
	}
	/* A full receive batch is successful network activity, not an error. */
	*test = (struct dhcp_handoff_test){
		.offer = "offered.bin",
		.rx_burst = true,
	};
	ut_assertok(env_set("autoload", "no"));
	ut_assertok(do_dhcp(NULL, 0, 1, argv));
	ut_asserteq(0, test->rx_error);
	ut_asserteq(ETH_PACKETS_BATCH_RECV, test->rx_packets);
	ut_asserteq(ETH_PACKETS_BATCH_RECV - 2, test->arp_replies);
	ut_asserteq_str("1.1.2.3", env_get("ipaddr"));
	ut_asserteq(0, test->tftp);
	ut_asserteq(0, test->nfs);
	ut_assertnull(netif_default);
	for (i = 0; i < ARRAY_SIZE(autoload); i++) {
		*test = (struct dhcp_handoff_test){ .offer = "offered.bin" };
		ut_assertok(env_set("autoload", autoload[i]));
		ut_assertok(dhcp_run(0, NULL, false));
		ut_asserteq(0, test->tftp);
		ut_asserteq(0, test->nfs);
		if (autoload[i])
			ut_asserteq_str(autoload[i], env_get("autoload"));
		else
			ut_assertnull(env_get("autoload"));
	}
	*test = (struct dhcp_handoff_test){ .offer = "offered.bin", .fail = true };
	ut_assertok(env_set("autoload", "NFS"));
	ut_asserteq(-ENOENT, dhcp_run(0x2000000, "explicit.bin", true));
	ut_asserteq_str("NFS", env_get("autoload"));
	ut_assert(!test->dhcp_alive);
	ut_assertnull(netif_default);
	*test = (struct dhcp_handoff_test){ .offer = "", .timeout = true };
	ut_asserteq(-ENOENT, dhcp_run(0x2000000, NULL, true));
	ut_asserteq(0, test->tftp);
	ut_asserteq_str("NFS", env_get("autoload"));
	ut_assertnull(netif_default);
	*test = (struct dhcp_handoff_test){ .offer = "offered.bin" };
	ut_asserteq(CMD_RET_FAILURE, do_dhcp(NULL, 0, 1, argv));
	ut_asserteq(IS_ENABLED(CONFIG_CMD_NFS) ? 1 : 0, test->nfs);
	ut_asserteq(0, test->tftp);
	ut_assert(!test->dhcp_alive);
	ut_assertnull(netif_default);

	/*
	 * The boot server can differ from the address server. Explicit NFS
	 * settings and BOOTP server policies still take precedence. Every
	 * transfer fails at the RPC request, exercising override cleanup too.
	 */
	for (i = 0; i < 4; i++) {
		const char *server = i == 1 || i == 2 ? "1.1.2.5" : NULL;
		char * const args[] = { "dhcp", "2000000", "1.1.2.6:/explicit.bin" };
		u32 expected;

		*test = (struct dhcp_handoff_test){
			.offer = "offered.bin",
			.boot_server = i == 3 ? 0 : 0x01010204,
		};
		ut_assertok(env_set("serverip", "1.1.2.9"));
		ut_assertok(env_set("nfsserverip", server));
		ut_asserteq(CMD_RET_FAILURE, do_dhcp(NULL, 0, i == 2 ? 3 : 1, args));
		ut_asserteq(IS_ENABLED(CONFIG_CMD_NFS) ? 1 : 0, test->nfs);
		expected = IS_ENABLED(CONFIG_BOOTP_SERVERIP) ||
			   IS_ENABLED(CONFIG_BOOTP_PREFER_SERVERIP) ? 0x01010209 :
			   i == 3 ? 0x01010202 : 0x01010204;
		if (i == 1)
			expected = 0x01010205;
		else if (i == 2)
			expected = 0x01010206;
		if (IS_ENABLED(CONFIG_CMD_NFS))
			ut_asserteq(expected, test->nfs_server);
		if (server)
			ut_asserteq_str(server, env_get("nfsserverip"));
		else
			ut_assertnull(env_get("nfsserverip"));
		ut_asserteq(0, test->tftp);
		ut_assert(!test->dhcp_alive);
		ut_assertnull(netif_default);
	}

	return 0;
}

static int dm_test_lwip_dhcp_handoff(struct unit_test_state *uts)
{
	static const char * const vars[] = {
		"ethact", "ipaddr", "netmask", "gatewayip", "serverip",
		"tftpserverip", "nfsserverip", "autoload", "autostart", "bootfile",
		"loadaddr", "dnsip", "dnsip2", "ntpserverip", "filesize", "fileaddr",
		"tftpdstp", "tftpsrcp",
	};
	struct dhcp_handoff_test test = {};
	char bootfile[sizeof(net_boot_file_name)];
	ulong load_addr = image_load_addr;
	char *saved[ARRAY_SIZE(vars)] = {};
	int i, ret = -ENOMEM;

	for (i = 0; i < ARRAY_SIZE(vars); i++) {
		const char *value = env_get(vars[i]);

		if (value) {
			saved[i] = strdup(value);
			if (!saved[i])
				goto out;
		}
	}
	memcpy(bootfile, net_boot_file_name, sizeof(bootfile));
	sandbox_eth_set_tx_handler(0, dhcp_handoff_tx);
	sandbox_eth_set_free_pkt_handler(0, dhcp_handoff_free_pkt);
	sandbox_eth_set_priv(0, &test);
	ret = dhcp_handoff_check(uts, &test);
	sandbox_eth_set_tx_handler(0, NULL);
	sandbox_eth_set_free_pkt_handler(0, NULL);
	sandbox_eth_set_priv(0, NULL);
	memcpy(net_boot_file_name, bootfile, sizeof(bootfile));
	image_load_addr = load_addr;
	for (i = 0; i < ARRAY_SIZE(vars); i++)
		if (env_set(vars[i], saved[i]))
			ret = -EINVAL;
out:
	for (i = 0; i < ARRAY_SIZE(vars); i++)
		free(saved[i]);

	return ret;
}

DM_TEST(dm_test_lwip_dhcp_handoff, UTF_SCAN_FDT);
