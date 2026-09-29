// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2026 James Hilliard <james.hilliard1@gmail.com> */

#include <command.h>
#include <dm.h>
#include <env.h>
#include <lwip-dhcp.h>
#include <malloc.h>
#include <net.h>
#include <asm/eth.h>
#include <asm/unaligned.h>
#include <dm/test.h>
#include <lwip/apps/sntp.h>
#include <lwip/dhcp.h>
#include <lwip/prot/dhcp.h>
#include <test/ut.h>

enum dhcp_reply_kind {
	DHCP_REPLY_BASIC,
	DHCP_REPLY_OVERLOAD,
	DHCP_REPLY_EMPTY,
	DHCP_REPLY_BAD_STRING,
	DHCP_REPLY_TRUNCATED,
	DHCP_REPLY_BAD_XID,
	DHCP_REPLY_BAD_SIZE,
	DHCP_REPLY_ZERO_OFFSET,
	DHCP_REPLY_BAD_NTP,
	DHCP_REPLY_ROOT_OVERFLOW,
};

struct dhcp_options_test {
	enum dhcp_reply_kind kind;
	unsigned int discover;
	unsigned int request;
	bool requested[256];
	char hostname[256];
	char vendor[256];
	bool chain_failed;
};

static bool dhcp_options_chain_check(struct dhcp_msg *reply, size_t len,
				     enum dhcp_reply_kind kind)
{
	static const unsigned int splits[] = { DHCP_FILE_OFS + 4, DHCP_OPTIONS_OFS + 3 };
	err_t expected = kind == DHCP_REPLY_BAD_STRING || kind == DHCP_REPLY_TRUNCATED ||
		kind == DHCP_REPLY_BAD_SIZE || kind == DHCP_REPLY_BAD_NTP ||
		kind == DHCP_REPLY_ROOT_OVERFLOW ? ERR_VAL : ERR_OK;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(splits); i++) {
		struct pbuf *head, *tail;
		err_t ret;

		head = pbuf_alloc(PBUF_RAW, splits[i], PBUF_RAM);
		tail = pbuf_alloc(PBUF_RAW, len - splits[i], PBUF_RAM);
		if (!head || !tail) {
			if (head)
				pbuf_free(head);
			if (tail)
				pbuf_free(tail);
			return false;
		}
		pbuf_take(head, reply, splits[i]);
		pbuf_take(tail, (u8 *)reply + splits[i], len - splits[i]);
		pbuf_cat(head, tail);
		/*
		 * Exercise the application parser with a value split between pbufs.
		 * Normal packet delivery below still exercises ACK acceptance.
		 */
		ret = net_lwip_dhcp_ack(netif_default, netif_dhcp_data(netif_default), head);
		pbuf_free(head);
		if (ret != expected)
			return false;
	}

	return true;
}

static u8 *dhcp_test_option(u8 *pos, u8 code, const void *value, u8 len)
{
	*pos++ = code;
	*pos++ = len;
	memcpy(pos, value, len);

	return pos + len;
}

static int dhcp_options_reply(struct udevice *dev, struct dhcp_msg *request,
			      u8 type, enum dhcp_reply_kind kind)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct dhcp_options_test *test = priv->priv;
	static const u8 server[] = { 1, 1, 2, 2 };
	static const u8 mask[] = { 255, 255, 255, 0 };
	static const u8 lease[] = { 0, 0, 0x0e, 0x10 };
	static const u8 size[] = { 0, 42 };
	static const u8 offset[] = { 0xff, 0xff, 0xf1, 0xf0 };
	static const u8 ntp[] = { 10, 0, 0, 1, 10, 0, 0, 2 };
	struct ethernet_hdr *eth;
	struct ip_udp_hdr *ip;
	struct dhcp_msg *reply;
	u8 *pos;
	size_t len;

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
	if (kind == DHCP_REPLY_BAD_XID)
		reply->xid ^= htonl(1);
	memcpy(reply->chaddr, request->chaddr, sizeof(reply->chaddr));
	put_unaligned_be32(0x01010203, &reply->yiaddr);
	put_unaligned_be32(0x01010204, &reply->siaddr);
	reply->cookie = htonl(DHCP_MAGIC_COOKIE);
	pos = reply->options;
	pos = dhcp_test_option(pos, DHCP_OPTION_SERVER_ID, server, sizeof(server));
	pos = dhcp_test_option(pos, DHCP_OPTION_SUBNET_MASK, mask, sizeof(mask));
	pos = dhcp_test_option(pos, DHCP_OPTION_LEASE_TIME, lease, sizeof(lease));
	if (kind == DHCP_REPLY_BASIC || kind == DHCP_REPLY_BAD_XID) {
		strcpy((char *)reply->file, "header.bin");
		pos = dhcp_test_option(pos, 12, "leased", 6);
		pos = dhcp_test_option(pos, 17, "/srv/root", 9);
		pos = dhcp_test_option(pos, 40, "example.test", 12);
		pos = dhcp_test_option(pos, 13, size, sizeof(size));
		pos = dhcp_test_option(pos, 2, offset, sizeof(offset));
		pos = dhcp_test_option(pos, 42, ntp, sizeof(ntp));
		/* A trailing NUL is legal and must not become part of the path. */
		pos = dhcp_test_option(pos, 67, "option.bin", sizeof("option.bin"));
	} else if (kind == DHCP_REPLY_OVERLOAD) {
		u8 overload = DHCP_OVERLOAD_SNAME_FILE;
		u8 *field;

		pos = dhcp_test_option(pos, 52, &overload, 1);
		pos = dhcp_test_option(pos, 17, "/srv", 4);
		pos = dhcp_test_option(pos, 67, "subdir/", 7);
		pos = dhcp_test_option(pos, 42, ntp, 1);
		field = dhcp_test_option(reply->file, 17, "/root", 5);
		field = dhcp_test_option(field, 67, "overloaded.bin", 14);
		field = dhcp_test_option(field, 42, ntp + 1, 2);
		*field = DHCP_OPTION_END;
		field = dhcp_test_option(reply->sname, 17, "/fs", 3);
		field = dhcp_test_option(field, 42, ntp + 3, sizeof(ntp) - 3);
		*field = DHCP_OPTION_END;
	} else if (kind == DHCP_REPLY_ZERO_OFFSET) {
		static const u8 zero[4];

		pos = dhcp_test_option(pos, 2, zero, sizeof(zero));
	} else if (kind != DHCP_REPLY_EMPTY) {
		/* None of these candidate values may escape a rejected ACK. */
		pos = dhcp_test_option(pos, 12, "poisoned", 8);
		pos = dhcp_test_option(pos, 17, "/wrong", 6);
		pos = dhcp_test_option(pos, 67, "wrong.bin", 9);
		if (kind == DHCP_REPLY_BAD_STRING)
			pos = dhcp_test_option(pos, 67, "\0bad", 4);
		if (kind == DHCP_REPLY_BAD_SIZE)
			pos = dhcp_test_option(pos, 13, size, 1);
		if (kind == DHCP_REPLY_BAD_NTP) {
			pos = dhcp_test_option(pos, 42, ntp, 1);
			pos = dhcp_test_option(pos, 42, ntp + 1, 2);
		}
		if (kind == DHCP_REPLY_ROOT_OVERFLOW) {
			u8 root[255];

			memset(root, 'r', sizeof(root));
			pos = dhcp_test_option(pos, 17, root, sizeof(root));
		}
	}
	/* Exercise custom options before the message-type option. */
	pos = dhcp_test_option(pos, DHCP_OPTION_MESSAGE_TYPE, &type, 1);
	if (kind == DHCP_REPLY_TRUNCATED) {
		*pos++ = 67;
		*pos++ = 20;
	}
	*pos++ = DHCP_OPTION_END;
	len = pos - (u8 *)reply;
	if (type == DHCP_ACK)
		test->chain_failed |= !dhcp_options_chain_check(reply, len, kind);
	ip->ip_hl_v = 0x45;
	ip->ip_len = htons(IP_UDP_HDR_SIZE + len);
	ip->ip_ttl = 64;
	ip->ip_p = IPPROTO_UDP;
	memcpy(&ip->ip_src, server, sizeof(server));
	ip->ip_dst.s_addr = htonl(0xffffffff);
	ip->udp_src = htons(67);
	ip->udp_dst = htons(68);
	ip->udp_len = htons(UDP_HDR_SIZE + len);
	ip->ip_sum = compute_ip_checksum(ip, IP_HDR_SIZE);
	priv->recv_packet_length[priv->recv_packets++] = ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + len;

	return 0;
}

static int dhcp_options_tx(struct udevice *dev, void *packet, unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct dhcp_options_test *test = priv->priv;
	struct ethernet_hdr *eth = packet;
	struct ip_udp_hdr *ip;
	struct dhcp_msg *msg;
	u8 *pos, *end, type = 0;
	int ret;

	if (!sandbox_eth_arp_req_to_reply(dev, packet, len))
		return 0;
	if (len < ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + DHCP_OPTIONS_OFS ||
	    ntohs(eth->et_protlen) != PROT_IP)
		return 0;
	ip = (void *)(eth + 1);
	if (ip->ip_p != IPPROTO_UDP || ntohs(ip->udp_dst) != 67)
		return 0;
	msg = (void *)(ip + 1);
	pos = msg->options;
	end = (u8 *)packet + len;
	while (pos < end && *pos != DHCP_OPTION_END) {
		u8 code = *pos++, size;

		if (code == DHCP_OPTION_PAD)
			continue;
		if (pos == end)
			return -EINVAL;
		size = *pos++;
		if (size > end - pos)
			return -EINVAL;
		if (code == DHCP_OPTION_MESSAGE_TYPE && size == 1)
			type = *pos;
		if (code == DHCP_OPTION_PARAMETER_REQUEST_LIST) {
			unsigned int i;

			for (i = 0; i < size; i++)
				test->requested[pos[i]] = true;
		}
		if (code == DHCP_OPTION_HOSTNAME || code == DHCP_OPTION_US) {
			char *str = code == DHCP_OPTION_HOSTNAME ? test->hostname : test->vendor;

			memcpy(str, pos, size);
			str[size] = 0;
		}
		pos += size;
	}
	if (type == DHCP_DISCOVER) {
		test->discover++;
		/* An offer must not publish its metadata when the ACK omits it. */
		return dhcp_options_reply(dev, msg, DHCP_OFFER, DHCP_REPLY_BASIC);
	}
	if (type != DHCP_REQUEST)
		return 0;
	test->request++;
	ret = dhcp_options_reply(dev, msg, DHCP_ACK, test->kind);
	if (!ret && test->kind >= DHCP_REPLY_BAD_STRING &&
	    test->kind != DHCP_REPLY_ZERO_OFFSET)
		ret = dhcp_options_reply(dev, msg, DHCP_ACK, DHCP_REPLY_EMPTY);

	return ret;
}

static int dhcp_options_check(struct unit_test_state *uts, struct dhcp_options_test *test)
{
	char * const argv[] = { "dhcp", "2000000", "explicit.bin" };
	char long_name[257];
	int kind;

	ut_assertok(env_set("ethact", "eth@10002000"));
	ut_assertok(env_set("autoload", "no"));
	ut_assertok(env_set("bootp_vci", "U-Boot.test"));
	for (kind = DHCP_REPLY_BASIC; kind <= DHCP_REPLY_ROOT_OVERFLOW; kind++) {
		bool have_ntp = IS_ENABLED(CONFIG_BOOTP_NTPSERVER) &&
			(kind == DHCP_REPLY_BASIC || kind == DHCP_REPLY_OVERLOAD);
		ip_addr_t old_ntp;

		if (CONFIG_IS_ENABLED(CMD_SNTP))
			ip_addr_copy(old_ntp, *sntp_getserver(0));
		if (kind == DHCP_REPLY_BAD_NTP && !IS_ENABLED(CONFIG_BOOTP_NTPSERVER))
			continue;
		if (kind == DHCP_REPLY_BAD_SIZE && !IS_ENABLED(CONFIG_BOOTP_BOOTFILESIZE))
			continue;
		if (kind == DHCP_REPLY_ROOT_OVERFLOW &&
		    (!IS_ENABLED(CONFIG_BOOTP_BOOTPATH) || CONFIG_BOOTP_MAX_ROOT_PATH_LEN > 261))
			continue;
		*test = (struct dhcp_options_test){ .kind = kind };
		ut_assertok(env_set("bootfile", "default.bin"));
		ut_assertok(env_set("hostname", "original"));
		ut_assertok(env_set("rootpath", "/original"));
		ut_assertok(env_set("domain", "original.test"));
		ut_assertok(env_set("ntpserverip", "9.8.7.6"));
		ut_assertok(env_set("timeoffset", "123"));
		ut_assertok(do_dhcp(NULL, 0, 1, argv));
		ut_asserteq(1, test->discover);
		ut_asserteq(1, test->request);
		ut_assert(!test->chain_failed);
		ut_assertnull(netif_default);
		ut_asserteq_str("1.1.2.3", env_get("ipaddr"));
		ut_asserteq_str("U-Boot.test", test->vendor);
		ut_asserteq_str(IS_ENABLED(CONFIG_BOOTP_SEND_HOSTNAME) ? "original" : "",
				test->hostname);
		ut_assert(test->requested[67]);
		ut_asserteq(IS_ENABLED(CONFIG_BOOTP_HOSTNAME), test->requested[12]);
		ut_asserteq(IS_ENABLED(CONFIG_BOOTP_BOOTPATH), test->requested[17]);
		ut_asserteq(IS_ENABLED(CONFIG_BOOTP_BOOTFILESIZE), test->requested[13]);
		ut_asserteq(IS_ENABLED(CONFIG_BOOTP_NISDOMAIN), test->requested[40]);
		ut_asserteq(IS_ENABLED(CONFIG_BOOTP_TIMEOFFSET), test->requested[2]);
		ut_asserteq(IS_ENABLED(CONFIG_BOOTP_NTPSERVER), test->requested[42]);
		ut_asserteq_str(kind == DHCP_REPLY_BASIC ? "option.bin" :
				kind == DHCP_REPLY_OVERLOAD ? "subdir/overloaded.bin" :
				"default.bin", env_get("bootfile"));
		ut_asserteq_str(kind == DHCP_REPLY_BASIC && IS_ENABLED(CONFIG_BOOTP_HOSTNAME) ?
				"leased" : "original", env_get("hostname"));
		ut_asserteq_str(!IS_ENABLED(CONFIG_BOOTP_BOOTPATH) ? "/original" :
				kind == DHCP_REPLY_BASIC ? "/srv/root" :
				kind == DHCP_REPLY_OVERLOAD ? "/srv/root/fs" :
				"/original", env_get("rootpath"));
		ut_asserteq_str(kind == DHCP_REPLY_BASIC && IS_ENABLED(CONFIG_BOOTP_NISDOMAIN) ?
				"example.test" : "original.test", env_get("domain"));
		ut_asserteq_str(have_ntp ?
				"10.0.0.1" : "9.8.7.6", env_get("ntpserverip"));
		if (CONFIG_IS_ENABLED(CMD_SNTP)) {
			if (have_ntp)
				ut_asserteq_str("10.0.0.1", ipaddr_ntoa(sntp_getserver(0)));
			else
				ut_assert(ip_addr_cmp(&old_ntp, sntp_getserver(0)));
		}
		ut_asserteq_str(!IS_ENABLED(CONFIG_BOOTP_TIMEOFFSET) ? "123" :
				kind == DHCP_REPLY_BASIC ? "-3600" :
				kind == DHCP_REPLY_ZERO_OFFSET ? "0" : "123",
				env_get("timeoffset"));
		ut_asserteq(kind == DHCP_REPLY_BASIC && IS_ENABLED(CONFIG_BOOTP_BOOTFILESIZE) ?
			   42 : 0, net_boot_file_expected_size_in_blocks);
	}
	*test = (struct dhcp_options_test){ .kind = DHCP_REPLY_BASIC };
	ut_assertok(do_dhcp(NULL, 0, 3, argv));
	ut_asserteq_str("explicit.bin", env_get("bootfile"));

	/* Maximum option values fit; a larger value is rejected before sending. */
	memset(long_name, 'x', sizeof(long_name));
	long_name[255] = 0;
	ut_assertok(env_set("bootp_vci", long_name));
	ut_assertok(env_set("hostname", long_name));
	*test = (struct dhcp_options_test){ .kind = DHCP_REPLY_EMPTY };
	ut_assertok(do_dhcp(NULL, 0, 1, argv));
	ut_asserteq_str(long_name, test->vendor);
	if (IS_ENABLED(CONFIG_BOOTP_SEND_HOSTNAME))
		ut_asserteq_str(long_name, test->hostname);
	long_name[255] = 'x';
	long_name[256] = 0;
	ut_assertok(env_set("bootp_vci", long_name));
	*test = (struct dhcp_options_test){};
	ut_asserteq(CMD_RET_FAILURE, do_dhcp(NULL, 0, 1, argv));
	ut_asserteq(0, test->discover);
	ut_assertnull(netif_default);

	return 0;
}

static int dm_test_lwip_dhcp_options(struct unit_test_state *uts)
{
	static const char * const vars[] = {
		"ethact", "autoload", "bootp_vci", "hostname", "domain", "rootpath",
		"bootfile", "timeoffset", "ntpserverip", "ipaddr", "netmask", "gatewayip",
		"serverip", "tftpserverip", "dnsip", "dnsip2",
	};
	struct dhcp_options_test test = {};
	ip_addr_t old_ntp;
	char bootfile[sizeof(net_boot_file_name)];
	u32 expected_size = net_boot_file_expected_size_in_blocks;
	char *saved[ARRAY_SIZE(vars)] = {};
	int i, ret = -ENOMEM;

	if (CONFIG_IS_ENABLED(CMD_SNTP))
		ip_addr_copy(old_ntp, *sntp_getserver(0));
	for (i = 0; i < ARRAY_SIZE(vars); i++) {
		const char *value = env_get(vars[i]);

		if (value) {
			saved[i] = strdup(value);
			if (!saved[i])
				goto out;
		}
	}
	memcpy(bootfile, net_boot_file_name, sizeof(bootfile));
	sandbox_eth_set_tx_handler(0, dhcp_options_tx);
	sandbox_eth_set_priv(0, &test);
	ret = dhcp_options_check(uts, &test);
	sandbox_eth_set_tx_handler(0, NULL);
	sandbox_eth_set_priv(0, NULL);
	memcpy(net_boot_file_name, bootfile, sizeof(bootfile));
	net_boot_file_expected_size_in_blocks = expected_size;
	for (i = 0; i < ARRAY_SIZE(vars); i++)
		if (env_set(vars[i], saved[i]))
			ret = -EINVAL;
out:
	if (CONFIG_IS_ENABLED(CMD_SNTP))
		sntp_setserver(0, &old_ntp);
	for (i = 0; i < ARRAY_SIZE(vars); i++)
		free(saved[i]);

	return ret;
}

DM_TEST(dm_test_lwip_dhcp_options, UTF_SCAN_FDT);
