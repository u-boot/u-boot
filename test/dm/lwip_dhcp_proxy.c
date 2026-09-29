// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2026 James Hilliard <james.hilliard1@gmail.com> */

#include <command.h>
#include <console.h>
#include <dm.h>
#include <efi_loader.h>
#include <env.h>
#include <malloc.h>
#include <net.h>
#include <time.h>
#include <asm/eth.h>
#include <asm/unaligned.h>
#include <dm/test.h>
#include <lwip/dhcp.h>
#include <lwip/prot/dhcp.h>
#include <lwip/timeouts.h>
#include <test/ut.h>

enum proxy_case {
	PROXY_BEFORE,
	PROXY_DURING,
	PROXY_AFTER,
	PROXY_LATE,
	PROXY_HEADER_FILE,
	PROXY_NO_SIADDR,
	PROXY_SECOND,
	PROXY_NONE,
	PROXY_BAD_XID,
	PROXY_BAD_MAC,
	PROXY_BAD_PORT,
	PROXY_BAD_COOKIE,
	PROXY_BAD_LENGTH,
	PROXY_BAD_VENDOR,
	PROXY_BAD_SERVER,
	PROXY_BAD_HTYPE,
	PROXY_BAD_HLEN,
	PROXY_MENU_ONLY,
	PROXY_NAK_RESTART,
	PROXY_WRONG_ACK_SERVER,
	PROXY_IGNORED_ACK,
	PROXY_RENEW_BAD_OPTION,
	PROXY_RENEW_NO_SERVER,
	PROXY_NO_LEASE,
	PROXY_CANCEL,
	PROXY_CASES,
};

struct proxy_test {
	enum proxy_case kind;
	struct udevice *dev;
	struct dhcp_msg request;
	struct efi_pxe_packet ack;
	struct efi_pxe_packet proxy;
	u32 xid;
	unsigned int discovers;
	unsigned int requests;
	bool wrong_requested_ip;
	bool delayed_bound;
	int delayed_error;
};

static u8 *proxy_option(u8 *p, u8 code, const void *data, u8 len)
{
	*p++ = code;
	*p++ = len;
	memcpy(p, data, len);

	return p + len;
}

static int proxy_reply(struct proxy_test *test, struct dhcp_msg *request, u8 type, bool proxy)
{
	struct eth_sandbox_priv *priv = dev_get_priv(test->dev);
	static const u8 mask[] = { 255, 255, 255, 0 };
	static const u8 server[] = { 1, 1, 2, 2 };
	static const u8 proxy_server[] = { 1, 1, 2, 5 };
	static const u8 lease[] = { 0, 0, 0x0e, 0x10 };
	bool renewal = !proxy && type == DHCP_ACK && test->requests > 1 &&
		(test->kind == PROXY_RENEW_BAD_OPTION || test->kind == PROXY_RENEW_NO_SERVER);
	struct ethernet_hdr *eth;
	struct ip_udp_hdr *ip;
	struct dhcp_msg *reply;
	u8 *p;
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
	memcpy(reply->chaddr, request->chaddr, sizeof(reply->chaddr));
	reply->cookie = htonl(DHCP_MAGIC_COOKIE);
	if (!proxy && type != DHCP_NAK)
		put_unaligned_be32(0x01010203, &reply->yiaddr);
	put_unaligned_be32(proxy ? 0x01010205 : 0x01010204, &reply->siaddr);
	p = proxy_option(reply->options, 53, &type, 1);
	if (!renewal || test->kind != PROXY_RENEW_NO_SERVER)
		p = proxy_option(p, 54, proxy ? proxy_server : server, sizeof(server));
	p = proxy_option(p, 1, mask, sizeof(mask));
	p = proxy_option(p, 51, lease, sizeof(lease));
	p = proxy_option(p, 3, server, sizeof(server));
	p = proxy_option(p, 6, server, sizeof(server));
	p = proxy_option(p, 17, proxy || renewal ? "/wrong" : "/lease", 6);
	if (renewal && test->kind == PROXY_RENEW_BAD_OPTION)
		p = proxy_option(p, 67, "bad\0file", 8);
	if (proxy) {
		p = proxy_option(p, 60, test->kind == PROXY_BAD_VENDOR ? "NotPXE123" :
				 "PXEClient", 9);
		if (test->kind == PROXY_HEADER_FILE) {
			strcpy((char *)reply->file, "proxy.bin");
		} else if (test->kind != PROXY_MENU_ONLY) {
			p = proxy_option(p, 67, "proxy.bin", 9);
			p = proxy_option(p, 209, "pxelinux.cfg/proxy", 18);
		}
		if (test->kind == PROXY_BAD_XID)
			reply->xid ^= htonl(1);
		if (test->kind == PROXY_BAD_MAC)
			reply->chaddr[0] ^= 1;
		if (test->kind == PROXY_BAD_COOKIE)
			reply->cookie ^= htonl(1);
		if (test->kind == PROXY_BAD_HTYPE)
			reply->htype = 2;
		if (test->kind == PROXY_BAD_HLEN)
			reply->hlen = 5;
		if (test->kind == PROXY_BAD_SERVER)
			put_unaligned_be32(0xe0000001, &reply->siaddr);
		if (test->kind == PROXY_NO_SIADDR || test->kind == PROXY_MENU_ONLY)
			put_unaligned_be32(0, &reply->siaddr);
		if (test->kind == PROXY_BAD_LENGTH) {
			*p++ = 67;
			*p++ = 30;
		}
	} else if (test->kind != PROXY_IGNORED_ACK) {
		/* Also exercise the fixed field: a later proxy must not clear it. */
		strcpy((char *)reply->file, renewal ? "rejected.bin" : "lease.bin");
	}
	*p++ = DHCP_OPTION_END;
	len = p - (u8 *)reply;
	if (proxy) {
		memset(&test->proxy, 0, sizeof(test->proxy));
		memcpy(&test->proxy, reply, len);
	} else if (type == DHCP_ACK && !renewal) {
		memset(&test->ack, 0, sizeof(test->ack));
		memcpy(&test->ack, reply, len);
	}
	ip->ip_hl_v = 0x45;
	ip->ip_len = htons(IP_UDP_HDR_SIZE + len);
	ip->ip_ttl = 64;
	ip->ip_p = IPPROTO_UDP;
	memcpy(&ip->ip_src, proxy ? proxy_server : server, sizeof(server));
	ip->ip_dst.s_addr = htonl(0xffffffff);
	ip->udp_src = htons(proxy && test->kind == PROXY_BAD_PORT ? 4011 : 67);
	ip->udp_dst = htons(68);
	ip->udp_len = htons(UDP_HDR_SIZE + len);
	ip->ip_sum = compute_ip_checksum(ip, IP_HDR_SIZE);
	priv->recv_packet_length[priv->recv_packets++] = ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + len;

	return 0;
}

static void proxy_delayed(void *arg)
{
	struct proxy_test *test = arg;

	test->delayed_bound = dhcp_supplied_address(netif_default);
	if (test->kind == PROXY_CANCEL)
		console_in_puts("\x03");
	else if (test->kind == PROXY_RENEW_BAD_OPTION || test->kind == PROXY_RENEW_NO_SERVER)
		/* Exercise renewal during the proxy wait without changing timers. */
		test->delayed_error = test->delayed_bound ? dhcp_renew(netif_default) : ERR_VAL;
	else
		test->delayed_error = proxy_reply(test, &test->request, DHCP_OFFER, true);
}

static int proxy_tx(struct udevice *dev, void *packet, unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct proxy_test *test = priv->priv;
	struct ethernet_hdr *eth = packet;
	struct ip_udp_hdr *ip = (void *)(eth + 1);
	struct dhcp_msg *msg = (void *)(ip + 1);
	u8 *p = msg->options, *end = (u8 *)packet + len, type = 0;
	int ret;

	if (!sandbox_eth_arp_req_to_reply(dev, packet, len))
		return 0;
	if (len < ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + DHCP_OPTIONS_OFS ||
	    eth->et_protlen != htons(PROT_IP) || ip->ip_p != IPPROTO_UDP ||
	    ip->udp_dst != htons(67))
		return 0;
	test->dev = dev;
	while (p < end && *p != 255) {
		u8 code = *p++, size;

		if (!code)
			continue;
		if (p == end)
			return -EINVAL;
		size = *p++;
		if (size > end - p)
			return -EINVAL;
		if (code == 53 && size == 1)
			type = *p;
		if (code == 50 && (size != 4 || get_unaligned_be32(p) != 0x01010203))
			test->wrong_requested_ip = true;
		p += size;
	}
	if (type == DHCP_DISCOVER) {
		test->discovers++;
		test->xid = msg->xid;
		if (test->kind == PROXY_BEFORE || test->kind == PROXY_NO_LEASE ||
		    (test->kind == PROXY_NAK_RESTART && test->discovers == 1)) {
			ret = proxy_reply(test, msg, DHCP_OFFER, true);
			if (ret)
				return ret;
		}
		if (test->kind == PROXY_NO_LEASE) {
			timer_test_add_offset(10001);
			return 0;
		}
		return proxy_reply(test, msg, DHCP_OFFER, false);
	}
	if (type != DHCP_REQUEST)
		return 0;
	test->requests++;
	if (test->kind == PROXY_NAK_RESTART && test->requests == 1)
		return proxy_reply(test, msg, DHCP_NAK, false);
	if (test->kind == PROXY_DURING) {
		ret = proxy_reply(test, msg, DHCP_OFFER, true);
		if (ret)
			return ret;
	}
	if (test->kind == PROXY_WRONG_ACK_SERVER) {
		struct dhcp_msg *bad;

		ret = proxy_reply(test, msg, DHCP_ACK, true);
		if (ret)
			return ret;
		bad = (void *)(priv->recv_packet_buffer[priv->recv_packets - 1] +
			       ETHER_HDR_SIZE + IP_UDP_HDR_SIZE);
		put_unaligned_be32(0x01010209, &bad->yiaddr);
	}
	ret = proxy_reply(test, msg, DHCP_ACK, false);
	if (ret)
		return ret;
	if (test->kind == PROXY_IGNORED_ACK) {
		struct dhcp_msg *bad;

		/*
		 * A matching MAC/XID alone does not make this a selected ACK.
		 * Deliver it after binding, while the proxy wait still polls.
		 */
		ret = proxy_reply(test, msg, DHCP_ACK, true);
		if (ret)
			return ret;
		bad = (void *)(priv->recv_packet_buffer[priv->recv_packets - 1] +
			       ETHER_HDR_SIZE + IP_UDP_HDR_SIZE);
		put_unaligned_be32(0x01010209, &bad->yiaddr);
		strcpy((char *)bad->file, "ignored.bin");
		return 0;
	}
	if (test->kind == PROXY_SECOND) {
		struct efi_pxe_packet first;
		struct dhcp_msg *second;

		ret = proxy_reply(test, msg, DHCP_OFFER, true);
		if (ret)
			return ret;
		first = test->proxy;
		ret = proxy_reply(test, msg, DHCP_OFFER, true);
		if (ret)
			return ret;
		second = (void *)(priv->recv_packet_buffer[priv->recv_packets - 1] +
				  ETHER_HDR_SIZE + IP_UDP_HDR_SIZE);
		put_unaligned_be32(0x01010206, &second->siaddr);
		test->proxy = first;
		return 0;
	}
	if (test->kind == PROXY_LATE || test->kind == PROXY_CANCEL) {
		memcpy(&test->request, msg, DHCP_OPTIONS_OFS);
		sys_timeout(5, proxy_delayed, test);
	} else if ((test->kind == PROXY_RENEW_BAD_OPTION ||
		    test->kind == PROXY_RENEW_NO_SERVER) && test->requests == 1) {
		sys_timeout(5, proxy_delayed, test);
	} else if (test->kind == PROXY_AFTER || test->kind == PROXY_HEADER_FILE ||
		   test->kind == PROXY_NO_SIADDR ||
		   (test->kind >= PROXY_BAD_XID && test->kind <= PROXY_MENU_ONLY)) {
		return proxy_reply(test, msg, DHCP_OFFER, true);
	}

	return 0;
}

static int proxy_check(struct unit_test_state *uts, struct proxy_test *test)
{
	struct efi_pxe_mode *mode = NULL;
	char * const argv[] = { "dhcp", "2000000", "explicit.bin" };
	bool enabled = IS_ENABLED(CONFIG_SERVERIP_FROM_PROXYDHCP);
	unsigned int delay = 0;
	int kind;

	if (enabled)
		delay = IF_ENABLED_INT(CONFIG_SERVERIP_FROM_PROXYDHCP,
				       CONFIG_SERVERIP_FROM_PROXYDHCP_DELAY_MS);
	ut_assertok(env_set("ethact", "eth@10002000"));
	ut_assertok(env_set("autoload", "no"));
	ut_assertok(env_set("bootpretransmitperiodinit", "5000"));
	ut_assertok(env_set("bootpretransmitperiodmax", "60000"));
	ut_assertok(env_set("bootpretryperiod", "10000"));
	ut_assertok(env_set("bootp_arch", NULL));
	ut_assertok(env_set("pxeuuid", NULL));
	ut_assertok(env_set("bootp_vci", "PXEClient:Arch:00022:UNDI:000000"));
	for (kind = 0; kind < PROXY_CASES; kind++) {
		bool selected = enabled && kind <= PROXY_SECOND &&
			(kind != PROXY_LATE || delay >= 10);
		bool cancel = kind == PROXY_CANCEL && enabled && delay >= 10;
		bool renewal = kind == PROXY_RENEW_BAD_OPTION || kind == PROXY_RENEW_NO_SERVER;
		unsigned long start, elapsed;
		int ret;

		if ((kind == PROXY_LATE || kind == PROXY_CANCEL) && delay && delay < 10)
			continue;
		if (renewal && (!enabled || delay < 10))
			continue;
		*test = (struct proxy_test){ .kind = kind };
		ut_assertok(env_set("serverip", "9.8.7.6"));
		ut_assertok(env_set("tftpserverip", "9.9.9.9"));
		ut_assertok(env_set("bootfile", "default.bin"));
		ut_assertok(env_set("rootpath", "/original"));
		clear_ctrlc();
		start = get_timer(0);
		ret = do_dhcp(NULL, 0, 1, argv);
		elapsed = get_timer(start);
		sys_untimeout(proxy_delayed, test);
		clear_ctrlc();
		ut_assertf(ret == (kind == PROXY_NO_LEASE || cancel ? CMD_RET_FAILURE : 0),
			   "case %d: return %d, delayed %d", kind, ret, test->delayed_bound);
		ut_assertnull(netif_default);
		ut_assert(!test->wrong_requested_ip);
		if (kind == PROXY_NO_LEASE || cancel) {
			ut_asserteq_str("default.bin", env_get("bootfile"));
			continue;
		}
		ut_asserteq(kind == PROXY_NAK_RESTART ? 2 : 1, test->discovers);
		ut_asserteq(kind == PROXY_NAK_RESTART || renewal ? 2 : 1, test->requests);
		ut_asserteq_str("1.1.2.3", env_get("ipaddr"));
		ut_asserteq_str("255.255.255.0", env_get("netmask"));
		ut_asserteq_str("1.1.2.2", env_get("gatewayip"));
		ut_asserteq_str(IS_ENABLED(CONFIG_BOOTP_BOOTPATH) ? "/lease" : "/original",
				env_get("rootpath"));
		ut_asserteq_str(selected ? "proxy.bin" :
				kind == PROXY_IGNORED_ACK ? "default.bin" : "lease.bin",
				env_get("bootfile"));
		if (IS_ENABLED(CONFIG_BOOTP_SERVERIP) ||
		    IS_ENABLED(CONFIG_BOOTP_PREFER_SERVERIP)) {
			ut_asserteq_str("9.8.7.6", env_get("serverip"));
			ut_assertnull(env_get("tftpserverip"));
		} else {
			ut_asserteq_str(selected ? "1.1.2.5" : "1.1.2.2", env_get("serverip"));
			ut_asserteq_str(selected ? "1.1.2.5" : "1.1.2.4", env_get("tftpserverip"));
		}
		if (selected && kind != PROXY_HEADER_FILE &&
		    IS_ENABLED(CONFIG_BOOTP_PXE_DHCP_OPTION))
			ut_asserteq_str("pxelinux.cfg/proxy", pxelinux_configfile);
		else
			ut_assertnull(pxelinux_configfile);
		if (kind == PROXY_NONE && enabled)
			ut_assert(elapsed >= delay && elapsed < delay + 1000);
		if ((kind == PROXY_LATE && selected) || renewal) {
			ut_assert(test->delayed_bound);
			ut_assertok(test->delayed_error);
		}
		if (CONFIG_IS_ENABLED(EFI_LOADER)) {
			bool valid, ack_equal, proxy_valid, proxy_equal;

			mode = calloc(1, sizeof(*mode));
			ut_assertnonnull(mode);
			efi_net_get_dhcp_ack(eth_get_dev(), mode);
			valid = mode->dhcp_ack_received;
			ack_equal = !memcmp(&mode->dhcp_ack, &test->ack, sizeof(test->ack));
			proxy_valid = mode->proxy_offer_received;
			proxy_equal = !memcmp(&mode->proxy_offer, &test->proxy,
					      sizeof(test->proxy));
			free(mode);
			ut_assert(valid && ack_equal);
			ut_asserteq(selected, proxy_valid);
			if (selected)
				ut_assert(proxy_equal);
		}
	}
	*test = (struct proxy_test){ .kind = PROXY_BEFORE };
	ut_assertok(do_dhcp(NULL, 0, 3, argv));
	ut_asserteq_str("explicit.bin", env_get("bootfile"));

	return 0;
}

static int dm_test_lwip_dhcp_proxy(struct unit_test_state *uts)
{
	static const char * const vars[] = {
		"ethact", "autoload", "bootfile", "bootp_arch", "bootp_vci", "pxeuuid",
		"serverip", "tftpserverip", "ipaddr", "netmask", "gatewayip", "dnsip", "dnsip2",
		"rootpath",
		"bootpretransmitperiodinit", "bootpretransmitperiodmax", "bootpretryperiod",
	};
	char *saved[ARRAY_SIZE(vars)] = {};
	char file[sizeof(net_boot_file_name)];
	char *config = pxelinux_configfile;
	u32 size = net_boot_file_expected_size_in_blocks;
	struct proxy_test *test;
	int ctrlc_state;
	int i, ret = -ENOMEM;

	test = calloc(1, sizeof(*test));
	ut_assertnonnull(test);
	for (i = 0; i < ARRAY_SIZE(vars); i++) {
		const char *value = env_get(vars[i]);

		if (value) {
			saved[i] = strdup(value);
			if (!saved[i])
				goto out;
		}
	}
	memcpy(file, net_boot_file_name, sizeof(file));
	pxelinux_configfile = NULL;
	sandbox_eth_set_tx_handler(0, proxy_tx);
	sandbox_eth_set_priv(0, test);
	ctrlc_state = disable_ctrlc(0);
	ret = proxy_check(uts, test);
	sys_untimeout(proxy_delayed, test);
	clear_ctrlc();
	disable_ctrlc(ctrlc_state);
	sandbox_eth_set_tx_handler(0, NULL);
	sandbox_eth_set_priv(0, NULL);
	memcpy(net_boot_file_name, file, sizeof(file));
	net_boot_file_expected_size_in_blocks = size;
	free(pxelinux_configfile);
	pxelinux_configfile = config;
	for (i = 0; i < ARRAY_SIZE(vars); i++)
		if (env_set(vars[i], saved[i]))
			ret = -EINVAL;
out:
	for (i = 0; i < ARRAY_SIZE(vars); i++)
		free(saved[i]);
	free(test);

	return ret;
}

DM_TEST(dm_test_lwip_dhcp_proxy, UTF_SCAN_FDT | UTF_CONSOLE);
