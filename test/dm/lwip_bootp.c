// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2026 James Hilliard <james.hilliard1@gmail.com> */

#include <command.h>
#include <console.h>
#include <dm.h>
#include <env.h>
#include <image.h>
#include <malloc.h>
#include <net.h>
#include <time.h>
#include <asm/eth.h>
#include <asm/unaligned.h>
#include <dm/test.h>
#include <lwip/apps/sntp.h>
#include <lwip/dns.h>
#include <lwip/prot/dhcp.h>
#include <lwip/udp.h>
#include <test/ut.h>

enum bootp_reply_kind {
	BOOTP_BASIC,
	BOOTP_PLAIN,
	BOOTP_EMPTY,
	BOOTP_OVERLOAD,
	BOOTP_BAD_XID,
	BOOTP_BAD_MAC,
	BOOTP_BAD_PORT,
	BOOTP_BAD_OP,
	BOOTP_BAD_HTYPE,
	BOOTP_BAD_HLEN,
	BOOTP_BAD_MASK,
	BOOTP_BAD_GATEWAY,
	BOOTP_BAD_OPTION,
	BOOTP_BAD_STRING,
	BOOTP_ZERO_IP,
	BOOTP_MULTICAST_IP,
	BOOTP_SHORT,
	BOOTP_DHCP_OFFER,
};

struct bootp_test {
	enum bootp_reply_kind kind;
	unsigned int requests;
	unsigned int bind_after;
	unsigned int tftp;
	u32 tftp_server;
	u32 xid;
	u16 secs;
	bool unicast;
	bool chain;
	bool cancel;
	bool fail_first;
	bool bad_request;
	bool port_busy;
	bool transfer_fail;
	bool no_server;
	bool requested[256];
	char filename[128];
};

static u8 *bootp_option(u8 *pos, u8 code, const void *value, u8 len)
{
	*pos++ = code;
	*pos++ = len;
	memcpy(pos, value, len);

	return pos + len;
}

static int bootp_reply(struct udevice *dev, struct dhcp_msg *request)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct bootp_test *test = priv->priv;
	static const u8 mask[] = { 255, 255, 255, 0 };
	static const u8 gateways[] = { 1, 1, 2, 1, 1, 1, 2, 254 };
	static const u8 dns[] = { 8, 8, 8, 8, 9, 9, 9, 9 };
	static const u8 ntp[] = { 10, 0, 0, 1 };
	static const u8 size[] = { 0, 42 };
	struct ethernet_hdr *eth;
	struct ip_udp_hdr *ip;
	struct dhcp_msg *reply;
	u8 *pos;
	size_t len;

	if (priv->recv_packets >= PKTBUFSRX)
		return -ENOSPC;
	eth = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memset(eth->et_dest, 0xff, ARP_HLEN);
	if (test->unicast)
		memcpy(eth->et_dest, request->chaddr, ARP_HLEN);
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
	put_unaligned_be32(test->no_server ? 0 : 0x01010204, &reply->siaddr);
	reply->cookie = htonl(DHCP_MAGIC_COOKIE);
	strcpy((char *)reply->file, "header.bin");
	pos = reply->options;
	if (test->kind != BOOTP_EMPTY) {
		pos = bootp_option(pos, 1, mask, sizeof(mask));
		pos = bootp_option(pos, 3, gateways, sizeof(gateways));
		pos = bootp_option(pos, 6, dns, sizeof(dns));
		pos = bootp_option(pos, 42, ntp, sizeof(ntp));
		pos = bootp_option(pos, 13, size, sizeof(size));
		pos = bootp_option(pos, 12, "bootp-host", 10);
		pos = bootp_option(pos, 17, "/srv/root", 9);
		pos = bootp_option(pos, 67, "option.bin", 10);
	} else {
		reply->file[0] = 0;
	}
	switch (test->kind) {
	case BOOTP_PLAIN:
		reply->cookie = 0;
		/* A proprietary vendor area is not parsed as RFC 1048 tags. */
		memset(reply->options, 0xa5, 60);
		break;
	case BOOTP_OVERLOAD: {
		u8 overload = DHCP_OVERLOAD_SNAME_FILE;
		u8 *field;

		pos = bootp_option(reply->options, 52, &overload, 1);
		pos = bootp_option(pos, 17, "/srv", 4);
		pos = bootp_option(pos, 3, gateways, 1);
		field = bootp_option(reply->file, 17, "/root", 5);
		field = bootp_option(field, 3, gateways + 1, 3);
		field = bootp_option(field, 67, "overloaded.bin", 14);
		*field = DHCP_OPTION_END;
		field = bootp_option(reply->sname, 1, mask, sizeof(mask));
		*field = DHCP_OPTION_END;
		break;
	}
	case BOOTP_BAD_XID:
		reply->xid ^= htonl(1);
		break;
	case BOOTP_BAD_MAC:
		reply->chaddr[0] ^= 1;
		break;
	case BOOTP_BAD_OP:
		reply->op = DHCP_BOOTREQUEST;
		break;
	case BOOTP_BAD_HTYPE:
		reply->htype = 2;
		break;
	case BOOTP_BAD_HLEN:
		reply->hlen = 0;
		break;
	case BOOTP_BAD_MASK:
		pos = bootp_option(pos, 1, mask, 1);
		break;
	case BOOTP_BAD_GATEWAY:
		pos = bootp_option(pos, 3, gateways, 1);
		break;
	case BOOTP_BAD_OPTION:
		*pos++ = 17;
		*pos++ = 255;
		break;
	case BOOTP_BAD_STRING:
		pos = bootp_option(pos, 67, "bad\0file", 8);
		break;
	case BOOTP_ZERO_IP:
		put_unaligned_be32(0, &reply->yiaddr);
		break;
	case BOOTP_MULTICAST_IP:
		put_unaligned_be32(0xe0000001, &reply->yiaddr);
		break;
	case BOOTP_DHCP_OFFER: {
		u8 type = DHCP_OFFER;

		pos = bootp_option(pos, 53, &type, 1);
		break;
	}
	default:
		break;
	}
	*pos++ = DHCP_OPTION_END;
	len = max_t(size_t, 300, pos - (u8 *)reply);
	if (test->kind == BOOTP_SHORT)
		len = 299;
	ip->ip_hl_v = 0x45;
	ip->ip_len = htons(IP_UDP_HDR_SIZE + len);
	ip->ip_ttl = 64;
	ip->ip_p = IPPROTO_UDP;
	/* A relay's source must not replace the siaddr boot server. */
	ip->ip_src.s_addr = htonl(0x01010202);
	ip->ip_dst.s_addr = htonl(test->unicast ? 0x01010203 : 0xffffffff);
	ip->udp_src = htons(test->kind == BOOTP_BAD_PORT ? 1234 : 67);
	ip->udp_dst = htons(68);
	ip->udp_len = htons(UDP_HDR_SIZE + len);
	ip->ip_sum = compute_ip_checksum(ip, IP_HDR_SIZE);
	len += ETHER_HDR_SIZE + IP_UDP_HDR_SIZE;
	if (test->chain) {
		unsigned int split = ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + DHCP_FILE_OFS + 4;
		struct pbuf *head = pbuf_alloc(PBUF_RAW, split, PBUF_RAM);
		struct pbuf *tail = pbuf_alloc(PBUF_RAW, len - split, PBUF_RAM);

		if (!head || !tail) {
			if (head)
				pbuf_free(head);
			if (tail)
				pbuf_free(tail);
			return -ENOMEM;
		}
		pbuf_take(head, eth, split);
		pbuf_take(tail, (u8 *)eth + split, len - split);
		pbuf_cat(head, tail);
		return netif_default->input(head, netif_default);
	}
	priv->recv_packet_length[priv->recv_packets++] = len;

	return 0;
}

static int bootp_tx(struct udevice *dev, void *packet, unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct bootp_test *test = priv->priv;
	struct ethernet_hdr *eth = packet;
	struct ip_udp_hdr *ip;
	struct dhcp_msg *msg;
	struct udp_pcb *pcb;
	u8 *pos, *end;

	if (!sandbox_eth_arp_req_to_reply(dev, packet, len))
		return 0;
	if (len < ETHER_HDR_SIZE + IP_UDP_HDR_SIZE || ntohs(eth->et_protlen) != PROT_IP)
		return 0;
	ip = (void *)(eth + 1);
	if (ip->ip_p != IPPROTO_UDP)
		return 0;
	if (ntohs(ip->udp_dst) == 69) {
		struct ethernet_hdr *reply_eth;
		struct ip_udp_hdr *reply_ip;
		u8 *data;

		test->tftp++;
		test->tftp_server = ntohl(ip->ip_dst.s_addr);
		strlcpy(test->filename, (char *)(ip + 1) + 2, sizeof(test->filename));
		for (pcb = udp_pcbs; pcb; pcb = pcb->next)
			if (pcb->local_port == 68)
				test->port_busy = true;
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
		put_unaligned_be16(test->transfer_fail ? 5 : 3, data);
		put_unaligned_be16(1, data + 2);
		data[4] = 'x';
		data[5] = 0;
		priv->recv_packet_length[priv->recv_packets++] =
			ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + 6;
		return 0;
	}
	if (ntohs(ip->udp_dst) != 67)
		return 0;
	if (len < ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + 300)
		return -EINVAL;
	msg = (void *)(ip + 1);
	test->requests++;
	if (test->requests == 1)
		test->xid = msg->xid;
	test->secs = ntohs(msg->secs);
	test->bad_request |= msg->op != DHCP_BOOTREQUEST || msg->htype != 1 ||
		msg->hlen != 6 || msg->hops || msg->flags || msg->ciaddr.addr ||
		msg->yiaddr.addr || msg->siaddr.addr || msg->giaddr.addr ||
		ntohs(ip->udp_src) != 68 || ip->ip_src.s_addr ||
		ip->ip_dst.s_addr != htonl(0xffffffff) ||
		ntohl(msg->cookie) != DHCP_MAGIC_COOKIE;
	if (!test->fail_first)
		test->bad_request |= test->xid != msg->xid;
	pos = msg->options;
	end = (u8 *)msg + ntohs(ip->udp_len) - UDP_HDR_SIZE;
	while (pos < end && *pos != DHCP_OPTION_END) {
		u8 code = *pos++;
		u8 size;

		if (!code)
			continue;
		if (pos == end || end - pos - 1 < *pos) {
			test->bad_request = true;
			break;
		}
		size = *pos++;
		test->requested[code] = true;
		test->bad_request |= code == 53 || code == 54 || code == 51 || code == 55;
		pos += size;
	}
	if (test->cancel) {
		console_in_puts("\x03");
		return 0;
	}
	if ((test->fail_first && !dev_seq(dev)) || test->requests < test->bind_after) {
		timer_test_add_offset(1000);
		return 0;
	}

	return bootp_reply(dev, msg);
}

static int bootp_check(struct unit_test_state *uts, struct bootp_test *test)
{
	char * const argv[] = { "bootp", "2000000", "explicit.bin" };
	struct net_lwip_ctx other = {};
	struct udp_pcb *occupied;
	char name[256];
	int i, ret;

	ut_assertnonnull(find_cmd("bootp"));
	ut_assertok(env_set("ethact", "eth@10002000"));
	ut_assertok(env_set("ethrotate", "no"));
	ut_assertok(env_set("netretry", "no"));
	ut_assertok(env_set("autoload", "no"));
	ut_assertok(env_set("autostart", "no"));
	ut_assertok(env_set("bootpretryperiod", "0"));
	ut_assertok(env_set("bootpretransmitperiodinit", "250"));
	ut_assertok(env_set("bootpretransmitperiodmax", "2000"));
	ut_assertok(env_set("bootp_vci", "U-Boot"));
	ut_assertok(env_set("tftpdstp", NULL));
	ut_assertok(env_set("tftpsrcp", NULL));
	for (i = BOOTP_BASIC; i <= BOOTP_OVERLOAD; i++) {
		*test = (struct bootp_test){ .kind = i, .unicast = i & 1, .chain = i & 2 };
		ut_assertok(env_set("bootfile", "default.bin"));
		ut_assertok(env_set("netmask", "255.0.0.0"));
		ut_assertok(env_set("gatewayip", "1.1.2.254"));
		ut_assertok(env_set("serverip", NULL));
		ut_assertok(do_bootp(NULL, 0, 1, argv));
		ut_asserteq(1, test->requests);
		ut_assert(!test->bad_request);
		ut_asserteq_str("1.1.2.3", env_get("ipaddr"));
		ut_asserteq_str(i == BOOTP_BASIC ? "option.bin" : i == BOOTP_PLAIN ?
				"header.bin" : i == BOOTP_EMPTY ? "default.bin" :
				"overloaded.bin", env_get("bootfile"));
		ut_asserteq_str(i == BOOTP_PLAIN || i == BOOTP_EMPTY ? "255.0.0.0" :
				"255.255.255.0", env_get("netmask"));
		ut_asserteq_str(i == BOOTP_PLAIN || i == BOOTP_EMPTY ? "1.1.2.254" :
				"1.1.2.1", env_get("gatewayip"));
		if (!IS_ENABLED(CONFIG_BOOTP_SERVERIP))
			ut_asserteq_str("1.1.2.4", env_get("serverip"));
		ut_assertnull(netif_default);
	}
	*test = (struct bootp_test){};
	ut_assertok(do_bootp(NULL, 0, 3, argv));
	ut_asserteq_str("explicit.bin", env_get("bootfile"));
	if (IS_ENABLED(CONFIG_BOOTP_BOOTPATH))
		ut_asserteq_str("/srv/root", env_get("rootpath"));
	if (IS_ENABLED(CONFIG_BOOTP_HOSTNAME))
		ut_asserteq_str("bootp-host", env_get("hostname"));
	if (IS_ENABLED(CONFIG_BOOTP_DNS))
		ut_asserteq_str("8.8.8.8", env_get("dnsip"));
	if (IS_ENABLED(CONFIG_BOOTP_DNS2))
		ut_asserteq_str("9.9.9.9", env_get("dnsip2"));
	if (IS_ENABLED(CONFIG_BOOTP_NTPSERVER))
		ut_asserteq_str("10.0.0.1", env_get("ntpserverip"));
	ut_asserteq(IS_ENABLED(CONFIG_BOOTP_BOOTFILESIZE) ? 42 : 0,
		    net_boot_file_expected_size_in_blocks);
	ut_asserteq(IS_ENABLED(CONFIG_BOOTP_BOOTPATH), test->requested[17]);
	ut_asserteq(IS_ENABLED(CONFIG_BOOTP_SUBNETMASK), test->requested[1]);
	ut_asserteq(IS_ENABLED(CONFIG_BOOTP_GATEWAY), test->requested[3]);
	ut_asserteq(IS_ENABLED(CONFIG_BOOTP_DNS), test->requested[6]);

	/* An omitted boot server must not create a zero-valued environment entry. */
	for (i = BOOTP_BASIC; i <= BOOTP_PLAIN; i++) {
		*test = (struct bootp_test){ .kind = i, .no_server = true };
		ut_assertok(env_set("serverip", NULL));
		ut_assertok(env_set("tftpserverip", "1.1.2.8"));
		ut_assertok(do_bootp(NULL, 0, 1, argv));
		ut_asserteq(1, test->requests);
		ut_assertnull(env_get("serverip"));
		ut_assertnull(env_get("tftpserverip"));
		ut_assertnull(netif_default);
	}

	/* Wrong clients, malformed options and DHCP offers must publish nothing. */
	for (i = BOOTP_BAD_XID; i <= BOOTP_DHCP_OFFER; i++) {
		*test = (struct bootp_test){ .kind = i };
		ut_assertok(env_set("bootfile", "kept.bin"));
		ut_assertok(env_set("ipaddr", "10.0.0.9"));
		ut_asserteq(CMD_RET_FAILURE, do_bootp(NULL, 0, 1, argv));
		ut_asserteq_str("kept.bin", env_get("bootfile"));
		ut_asserteq_str("10.0.0.9", env_get("ipaddr"));
		ut_assertnull(netif_default);
	}
	/* The largest hostname and VCI must still leave room for request tags. */
	memset(name, 'x', sizeof(name) - 1);
	name[sizeof(name) - 1] = 0;
	ut_assertok(env_set("bootp_vci", name));
	ut_assertok(env_set("hostname", name));
	*test = (struct bootp_test){};
	ut_assertok(do_bootp(NULL, 0, 1, argv));
	ut_assert(!test->bad_request);
	ut_assertok(env_set("bootp_vci", "U-Boot"));

	ut_assertok(env_set("bootpretryperiod", "4000"));
	*test = (struct bootp_test){ .bind_after = 3 };
	ut_assertok(do_bootp(NULL, 0, 1, argv));
	ut_asserteq(3, test->requests);
	ut_assert(test->secs >= 2);
	ut_assert(!test->bad_request);
	ut_assertok(env_set("bootpretryperiod", "1000"));
	ut_assertok(env_set("netretry", "once"));
	ut_assertok(env_set("ethrotate", NULL));
	*test = (struct bootp_test){ .fail_first = true };
	ut_assertok(do_bootp(NULL, 0, 1, argv));
	ut_asserteq(2, test->requests);
	ut_asserteq_str("eth@10003000", env_get("ethact"));
	ut_asserteq_str("1.1.2.3", env_get("ipaddr5"));
	ut_assertok(env_set("ethact", "eth@10002000"));
	ut_assertok(env_set("ethrotate", "no"));
	ut_assertok(env_set("netretry", "yes"));
	*test = (struct bootp_test){ .cancel = true };
	ut_asserteq(CMD_RET_FAILURE, do_bootp(NULL, 0, 1, argv));
	clear_ctrlc();
	ut_asserteq(1, test->requests);
	ut_assertnull(netif_default);
	ut_assertok(env_set("netretry", "no"));

	/* An occupied port is not stolen, and a flexible runtime user survives. */
	ut_assertok(net_lwip_start(&other, NET_LWIP_ADDR_ENV_FLEXIBLE));
	occupied = udp_new();
	ut_assertnonnull(occupied);
	ret = udp_bind(occupied, IP4_ADDR_ANY, 68);
	if (ret) {
		udp_remove(occupied);
		net_lwip_stop(&other);
		return ret;
	}
	*test = (struct bootp_test){};
	ret = do_bootp(NULL, 0, 1, argv);
	i = other.netif == netif_default && occupied->local_port == 68;
	udp_remove(occupied);
	net_lwip_stop(&other);
	ut_asserteq(CMD_RET_FAILURE, ret);
	ut_assert(i);
	ut_asserteq(0, test->requests);
	ut_assertnull(netif_default);

	ut_assertok(env_set("autoload", "yes"));
	ut_assertok(env_set("serverip", "1.1.2.4"));
	*test = (struct bootp_test){};
	ut_asserteq(IS_ENABLED(CONFIG_CMD_TFTPBOOT) ? CMD_RET_SUCCESS : CMD_RET_FAILURE,
		    do_bootp(NULL, 0, 3, argv));
	ut_asserteq(IS_ENABLED(CONFIG_CMD_TFTPBOOT) ? 1 : 0, test->tftp);
	ut_assert(!test->port_busy);
	if (test->tftp)
		ut_asserteq_str("explicit.bin", test->filename);
	ut_assertnull(netif_default);
	*test = (struct bootp_test){ .transfer_fail = true };
	ut_asserteq(CMD_RET_FAILURE, do_bootp(NULL, 0, 3, argv));
	ut_assert(!test->port_busy);
	ut_assertnull(netif_default);
	/*
	 * Plain and RFC 1048 replies without siaddr retain the configured server,
	 * but must clear stale TFTP overrides before the download handoff.
	 */
	for (i = BOOTP_BASIC; i <= BOOTP_PLAIN; i++) {
		*test = (struct bootp_test){ .kind = i, .no_server = true };
		ut_assertok(env_set("serverip", "1.1.2.9"));
		ut_assertok(env_set("tftpserverip", "1.1.2.8"));
		ut_asserteq(IS_ENABLED(CONFIG_CMD_TFTPBOOT) ? CMD_RET_SUCCESS : CMD_RET_FAILURE,
			    do_bootp(NULL, 0, 3, argv));
		ut_asserteq(1, test->requests);
		ut_asserteq_str("1.1.2.9", env_get("serverip"));
		ut_assertnull(env_get("tftpserverip"));
		ut_asserteq(IS_ENABLED(CONFIG_CMD_TFTPBOOT) ? 1 : 0, test->tftp);
		if (test->tftp) {
			ut_asserteq(0x01010209, test->tftp_server);
			ut_asserteq_str("explicit.bin", test->filename);
		}
		ut_assert(!test->port_busy);
		ut_assertnull(netif_default);
	}

	return 0;
}

static int dm_test_lwip_bootp(struct unit_test_state *uts)
{
	static const char * const vars[] = {
		"ethact", "ethrotate", "netretry", "autoload", "autostart", "bootfile",
		"ipaddr", "netmask", "gatewayip", "ipaddr5", "netmask5", "gatewayip5",
		"serverip", "tftpserverip", "dnsip", "dnsip2", "rootpath", "hostname",
		"domain", "timeoffset", "ntpserverip", "bootp_vci", "tftpdstp", "tftpsrcp",
		"fileaddr", "filesize",
		"bootpretryperiod", "bootpretransmitperiodinit", "bootpretransmitperiodmax",
	};
	struct bootp_test test = {};
	char *saved[ARRAY_SIZE(vars)] = {};
	char file[sizeof(net_boot_file_name)];
	char *config = pxelinux_configfile;
	u32 size = net_boot_file_expected_size_in_blocks;
	ip_addr_t ntp;
	struct in_addr old_ip = net_ip;
	ulong load_addr = image_load_addr;
#if LWIP_DNS
	ip_addr_t dns[DNS_MAX_SERVERS];
#endif
	int i, ret = -ENOMEM, ctrlc_state;

	for (i = 0; i < ARRAY_SIZE(vars); i++) {
		const char *value = env_get(vars[i]);

		if (value) {
			saved[i] = strdup(value);
			if (!saved[i])
				goto out;
		}
	}
	if (CONFIG_IS_ENABLED(CMD_SNTP))
		ip_addr_copy(ntp, *sntp_getserver(0));
#if LWIP_DNS
	for (i = 0; i < ARRAY_SIZE(dns); i++)
		ip_addr_copy(dns[i], *dns_getserver(i));
#endif
	memcpy(file, net_boot_file_name, sizeof(file));
	pxelinux_configfile = NULL;
	ctrlc_state = disable_ctrlc(0);
	sandbox_eth_set_tx_handler(0, bootp_tx);
	sandbox_eth_set_tx_handler(1, bootp_tx);
	sandbox_eth_set_priv(0, &test);
	sandbox_eth_set_priv(1, &test);
	ret = bootp_check(uts, &test);
	clear_ctrlc();
	disable_ctrlc(ctrlc_state);
	sandbox_eth_set_tx_handler(0, NULL);
	sandbox_eth_set_tx_handler(1, NULL);
	sandbox_eth_set_priv(0, NULL);
	sandbox_eth_set_priv(1, NULL);
	memcpy(net_boot_file_name, file, sizeof(file));
	net_boot_file_expected_size_in_blocks = size;
	net_ip = old_ip;
	image_load_addr = load_addr;
	free(pxelinux_configfile);
	pxelinux_configfile = config;
	if (CONFIG_IS_ENABLED(CMD_SNTP))
		sntp_setserver(0, &ntp);
#if LWIP_DNS
	for (i = 0; i < ARRAY_SIZE(dns); i++)
		dns_setserver(i, &dns[i]);
#endif
	for (i = 0; i < ARRAY_SIZE(vars); i++)
		if (env_set(vars[i], saved[i]))
			ret = -EINVAL;
out:
	for (i = 0; i < ARRAY_SIZE(vars); i++)
		free(saved[i]);

	return ret;
}

DM_TEST(dm_test_lwip_bootp, UTF_SCAN_FDT | UTF_CONSOLE);
