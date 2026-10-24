// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2026 James Hilliard <james.hilliard1@gmail.com> */

#include <command.h>
#include <console.h>
#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <image.h>
#include <malloc.h>
#include <mapmem.h>
#include <net.h>
#include <time.h>
#include <asm/eth.h>
#include <asm/unaligned.h>
#include <dm/test.h>
#include <test/ut.h>
#if IS_ENABLED(CONFIG_NET_LWIP)
#include <lwip/timeouts.h>
#include <lwip/udp.h>
#endif

static const char tftp_test_data[] = "update data, not a bootable image";

#define TFTP_TEST_ADDR 0x2000000

enum tftp_test_probe {
	TFTP_PROBE_NONE,
	TFTP_PROBE_SILENT,
	TFTP_PROBE_DATA,
	TFTP_PROBE_OACK,
};

struct tftp_test_context {
	bool fail;
	bool empty;
	bool autostart;
	unsigned int requests;
	unsigned int arp_requests;
	unsigned int drop_arp;
	struct in_addr arp_target;
	bool abort_arp;
	enum tftp_test_probe probe;
	unsigned int probe_acks;
	ulong request_start;
	ulong ack_start;
	ulong ack_delay;
	unsigned int advertised_timeout;
	char filename[64];
	struct in_addr local_ip;
	u16 blocksize;
	u16 windowsize;
	u16 port;
};

static ulong tftp_test_option(const char *packet, size_t len, const char *option)
{
	const char *end = packet + len;
	const char *name = packet + 2;
	size_t size;
	int i;

	/* Skip the request's filename and mode before walking option pairs. */
	for (i = 0; i < 2; i++) {
		if (name >= end)
			return 0;
		size = strnlen(name, end - name);
		if (size == end - name)
			return 0;
		name += size + 1;
	}
	while (name < end) {
		const char *value;

		size = strnlen(name, end - name);
		if (size == end - name)
			break;
		value = name + size + 1;
		if (value >= end)
			break;
		size = strnlen(value, end - value);
		if (size == end - value)
			break;
		if (!strcmp(name, option))
			return simple_strtoul(value, NULL, 10);
		name = value + size + 1;
	}
	return 0;
}

/* Answer an RRQ with one short DATA block or a file-not-found error. */
static int tftp_test_tx_handler(struct udevice *dev, void *packet,
				unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct tftp_test_context *ctx = priv->priv;
	struct ethernet_hdr *eth = packet, *reply_eth;
	struct ip_udp_hdr *ip, *reply_ip;
	const char *data = ctx->fail ? "not found" : tftp_test_data;
	unsigned int data_len = ctx->empty ? 0 : strlen(data) + 1;
	unsigned int reply_len, header_len = 4;
	u16 opcode, reply_opcode = 3, block = 1;
	bool partial = false;
	u8 *payload;

	if (len >= ETHER_HDR_SIZE + ARP_HDR_SIZE &&
	    ntohs(eth->et_protlen) == PROT_ARP) {
		const u8 *arp = (void *)(eth + 1);

		/* Ignore gratuitous ARP: compare the sender and target IPv4 addresses. */
		if (get_unaligned_be16(arp + 6) == 1 &&
		    memcmp(arp + 14, arp + 24, 4)) {
			memcpy(&ctx->arp_target, arp + 24, 4);
			ctx->arp_requests++;
			if (ctx->arp_requests <= ctx->drop_arp) {
				if (ctx->abort_arp)
					console_in_puts("\x03");
				else if (IS_ENABLED(CONFIG_NET_LWIP))
					/* Advance after the client has armed its RRQ timer. */
					sandbox_eth_skip_timeout();
				else
					timer_test_add_offset(CONFIG_ARP_TIMEOUT + 1);
				return 0;
			}
		}
	}
	if (!sandbox_eth_arp_req_to_reply(dev, packet, len))
		return 0;
	if (len < ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + 2 ||
	    ntohs(eth->et_protlen) != PROT_IP)
		return 0;
	ip = (void *)(eth + 1);
	if (ip->ip_p != IPPROTO_UDP)
		return 0;
	opcode = get_unaligned_be16(ip + 1);
	if (opcode == 4 && ctx->probe >= TFTP_PROBE_DATA) {
		ctx->probe_acks++;
		if (ctx->probe_acks == 1) {
			ctx->ack_start = get_timer(0);
			return 0;
		}
		if (ctx->probe_acks != 2)
			return 0;
		ctx->ack_delay = get_timer(ctx->ack_start);
		block = ctx->probe == TFTP_PROBE_DATA ? 2 : 1;
	} else if (opcode == 1 || opcode == 2) {
		size_t request_len = len - ETHER_HDR_SIZE - IP_UDP_HDR_SIZE;

		if (!ctx->requests++)
			ctx->request_start = get_timer(0);
		strlcpy(ctx->filename, (char *)(ip + 1) + 2, sizeof(ctx->filename));
		ctx->local_ip = ip->ip_src;
		ctx->blocksize = tftp_test_option((char *)(ip + 1), request_len, "blksize");
		ctx->windowsize = tftp_test_option((char *)(ip + 1), request_len, "windowsize");
		ctx->port = ntohs(ip->udp_dst);
		ctx->advertised_timeout = tftp_test_option((char *)(ip + 1), request_len, "timeout");
		if (ctx->probe == TFTP_PROBE_SILENT)
			return 0;
		if (ctx->probe == TFTP_PROBE_DATA) {
			partial = true;
			data_len = 512;
		} else if (ctx->probe == TFTP_PROBE_OACK) {
			/* Consume an initial retry before switching timeout budgets. */
			if (ctx->requests == 1)
				return 0;
			reply_opcode = 6;
			header_len = 2;
			data = IS_ENABLED(CONFIG_NET_TFTP_VARS) ?
				"timeout\0" "1" : "timeout\0" "5";
			data_len = 10;
		}
		if (opcode == 2) {
			reply_opcode = 4;
			block = 0;
			data_len = 0;
		}
	} else if (ntohs(ip->udp_dst) == 1234 && opcode == 3 &&
		   len >= ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + 4) {
		reply_opcode = 4;
		block = get_unaligned_be16((u8 *)(ip + 1) + 2);
		data_len = 0;
	} else {
		return 0;
	}
	reply_len = ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + header_len + data_len;
	if (priv->recv_packets >= PKTBUFSRX)
		return -ENOSPC;

	ctx->autostart |= env_get_autostart();
	reply_eth = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(reply_eth->et_dest, eth->et_src, ARP_HLEN);
	memcpy(reply_eth->et_src, priv->fake_host_hwaddr, ARP_HLEN);
	reply_eth->et_protlen = htons(PROT_IP);

	reply_ip = (void *)(reply_eth + 1);
	memset(reply_ip, 0, IP_UDP_HDR_SIZE);
	reply_ip->ip_hl_v = 0x45;
	reply_ip->ip_len = htons(IP_UDP_HDR_SIZE + header_len + data_len);
	reply_ip->ip_ttl = 64;
	reply_ip->ip_p = IPPROTO_UDP;
	reply_ip->ip_src = ip->ip_dst;
	reply_ip->ip_dst = ip->ip_src;
	reply_ip->udp_src = htons(1234);
	reply_ip->udp_dst = ip->udp_src;
	reply_ip->udp_len = htons(UDP_HDR_SIZE + header_len + data_len);
	reply_ip->ip_sum = compute_ip_checksum(reply_ip, IP_HDR_SIZE);
	payload = (void *)(reply_ip + 1);
	put_unaligned_be16(ctx->fail ? 5 : reply_opcode, payload);
	if (header_len == 4)
		put_unaligned_be16(block, payload + 2);
	if (partial)
		memset(payload + header_len, 0xee, data_len);
	else
		memcpy(payload + header_len, data, data_len);
	priv->recv_packet_length[priv->recv_packets++] = reply_len;

	return 0;
}

static int tftp_test_probe_arp(struct unit_test_state *uts,
			       struct tftp_test_context *ctx)
{
	void *buffer = map_sysmem(TFTP_TEST_ADDR, sizeof(tftp_test_data));
	ulong elapsed;
	int i, ret, ctrlc_state;

	ut_assertok(env_set("autostart", "no"));
	ut_assertok(run_command("setenv gatewayip 1.1.2.1", 0));
	for (i = 0; i < 4; i++) {
		ut_assertok(run_command(i == 1 ? "setenv serverip 1.2.3.4" :
				       "setenv serverip 1.1.2.2", 0));
		memset(ctx, 0, sizeof(*ctx));
		ctx->drop_arp = i == 3 ? UINT_MAX : 1;
		ctx->probe = i == 2 ? TFTP_PROBE_SILENT : TFTP_PROBE_NONE;
		ret = tftpb_run_timeout(TFTP_TEST_ADDR, "update.bin", 100, 0);
		elapsed = get_timer(ctx->request_start);
		ut_assertnull(tftp_get_request_timeout());
		ut_asserteq(i < 2 ? 0 : -ENOENT, ret);
		ut_asserteq(i == 3 ? max(1, CONFIG_NET_RETRY_COUNT - 1) : 2,
			    ctx->arp_requests);
		ut_asserteq(i == 3 ? 0 : 1, ctx->requests);
		ut_asserteq(htonl(i == 1 ? 0x01010201 : 0x01010202),
			    ctx->arp_target.s_addr);
		if (i < 2) {
			ut_asserteq(sizeof(tftp_test_data), net_boot_file_size);
			ut_asserteq_mem(tftp_test_data, buffer, sizeof(tftp_test_data));
		} else if (i == 2) {
			/* ARP retries must not consume the server's response budget. */
			ut_assert(elapsed >= 100);
			ut_assert(elapsed < 900);
		}
	}
	/* Ctrl-C must also cancel the separate ARP wait. */
	memset(ctx, 0, sizeof(*ctx));
	ctx->drop_arp = UINT_MAX;
	ctx->abort_arp = true;
	ctrlc_state = disable_ctrlc(0);
	ret = tftpb_run_timeout(TFTP_TEST_ADDR, "update.bin", 100, 0);
	clear_ctrlc();
	disable_ctrlc(ctrlc_state);
	ut_asserteq(-ENOENT, ret);
	ut_asserteq(1, ctx->arp_requests);
	ut_asserteq(0, ctx->requests);
	ut_assertnull(tftp_get_request_timeout());

	/* Failed or cancelled ARP must not leave a request or timer behind. */
	memset(ctx, 0, sizeof(*ctx));
	ut_assertok(tftpb_run_timeout(TFTP_TEST_ADDR, "update.bin", 100, 0));
	ut_asserteq(1, ctx->requests);
	unmap_sysmem(buffer);
	return 0;
}

static int tftp_test_probe_timeout(struct unit_test_state *uts,
				   struct tftp_test_context *ctx)
{
	ulong elapsed;
	int i, ret;

	ut_assertok(env_set("autostart", "no"));
	ut_assertok(env_set("tftptimeout", "1000"));
	ut_assertok(env_set("tftptimeoutcountmax", "1"));
	for (i = 0; i < 4; i++) {
		memset(ctx, 0, sizeof(*ctx));
		ctx->probe = i < 2 ? TFTP_PROBE_SILENT :
			     i == 2 ? TFTP_PROBE_DATA : TFTP_PROBE_OACK;
		ret = tftpb_run_timeout(TFTP_TEST_ADDR, "update.bin", 100, i & 1 ? 2 : 0);
		elapsed = get_timer(ctx->request_start);
		ut_assertnull(tftp_get_request_timeout());
		ut_asserteq(i < 2 ? -ENOENT : 0, ret);
		ut_asserteq(i == 1 ? 3 : i == 3 ? 2 : 1, ctx->requests);
		ut_asserteq(IS_ENABLED(CONFIG_NET_TFTP_VARS) ? 1 : 5, ctx->advertised_timeout);
		if (i < 2) {
			ut_assert(elapsed >= (i ? 300 : 100));
			ut_assert(elapsed < 900);
		} else {
			ut_asserteq(3, ctx->probe_acks);
			ut_assert(ctx->ack_delay >= 1000);
		}
	}
	/* The next ordinary transfer must not inherit the short probe. */
	memset(ctx, 0, sizeof(*ctx));
	ctx->probe = TFTP_PROBE_OACK;
	ut_assertok(tftpb_run(TFTP_TEST_ADDR, "update.bin"));
	ut_assert(get_timer(ctx->request_start) >= 2000);
	ut_asserteq(2, ctx->requests);
	ut_asserteq(3, ctx->probe_acks);
	ut_asserteq_str("1000", env_get("tftptimeout"));
	ut_asserteq_str("1", env_get("tftptimeoutcountmax"));
	ut_asserteq(-EINVAL, tftpb_run_timeout(TFTP_TEST_ADDR, "update.bin", 0, 0));
	ut_asserteq(-EINVAL, tftpb_run_timeout(TFTP_TEST_ADDR, "update.bin", 100, -1));
	ut_asserteq(-EINVAL, tftpb_run_timeout(TFTP_TEST_ADDR, "update.bin", 1073741824, 0));
	return 0;
}

static int tftp_test_filenames(struct unit_test_state *uts,
			       struct tftp_test_context *ctx)
{
	static const char * const files[] = {
		"dir:file", "dir:one:two:file", "1.1.2.2:path:file",
	};
	static const char * const expected[] = {
		"dir:file", "dir:one:two:file", "path:file",
	};
	char command[100];
	int i;

	ut_assertok(env_set("autostart", "no"));
	for (i = 0; i < ARRAY_SIZE(files); i++) {
		memset(ctx, 0, sizeof(*ctx));
		snprintf(command, sizeof(command), "tftpboot 2000000 %s", files[i]);
		ut_assertok(run_command(command, 0));
		ut_asserteq(1, ctx->requests);
		ut_asserteq_str(expected[i], ctx->filename);
		ut_assertok(env_set("bootfile", files[i]));
		ut_assertok(run_command("tftpboot 2000000", 0));
		ut_asserteq_str(expected[i], ctx->filename);
	}
	if (IS_ENABLED(CONFIG_NET_LWIP)) {
		/* Keep the lwIP numeric-port extension, including filename colons. */
		ut_assertok(run_command("tftpboot 2000000 1.1.2.2:1069:path:to:file", 0));
		ut_asserteq(1069, ctx->port);
		ut_asserteq_str("path:to:file", ctx->filename);
		ut_assertok(run_command("tftpboot 2000000 :1069:path:to:file", 0));
		ut_asserteq_str("path:to:file", ctx->filename);
		ut_assertok(run_command("tftpboot 2000000 1.1.2.2::path:file", 0));
		ut_asserteq(69, ctx->port);
		ut_asserteq_str("path:file", ctx->filename);
		memset(ctx, 0, sizeof(*ctx));
		ut_asserteq(CMD_RET_FAILURE,
			    run_command("tftpboot 2000000 1.1.2.2:65536:file", 0));
		ut_asserteq(0, ctx->requests);
	}
	return 0;
}

#if IS_ENABLED(CONFIG_NET_LWIP)
static int tftp_test_options(struct unit_test_state *uts,
			     struct tftp_test_context *ctx)
{
	static const struct {
		const char *value;
		bool valid;
	} sizes[] = {
		{ NULL, true },
		{ "512", true },
		{ "4096", true },
		{ "65464", true },
		{ "1", false },
		{ "bad", false },
	};
	ulong rx_max = IP_REASSEMBLY ?
		min_t(ulong, 65464, IP_REASS_MAX_PBUFS * IP_FRAG_MTU_USABLE - 12) : 1468;
	bool vars = IS_ENABLED(CONFIG_NET_TFTP_VARS);
	ulong requested, expected;
	int i, put, ret;

	ut_assertok(env_set("autostart", "no"));
	ut_assertok(env_set("tftpwindowsize", "4"));
	for (put = 0; put <= IS_ENABLED(CONFIG_CMD_TFTPPUT); put++) {
		for (i = 0; i < ARRAY_SIZE(sizes); i++) {
			memset(ctx, 0, sizeof(*ctx));
			ut_assertok(env_set("tftpblocksize", sizes[i].value));
			ret = run_command(put ? "tftpput 2000000 20 file" :
					  "tftpboot 2000000 file", 0);
			if (vars && !sizes[i].valid) {
				ut_asserteq(CMD_RET_FAILURE, ret);
				ut_asserteq(0, ctx->requests);
				continue;
			}
			ut_assertok(ret);
			requested = vars && sizes[i].value ?
				simple_strtoul(sizes[i].value, NULL, 10) : CONFIG_TFTP_BLOCKSIZE;
			expected = min(requested, put ? 1468UL : rx_max);
			/* The default block size does not need an on-wire option. */
			ut_asserteq(expected == 512 ? 0 : expected, ctx->blocksize);
			expected = vars ? 4 : CONFIG_TFTP_WINDOWSIZE;
			ut_asserteq(put || expected == 1 ? 0 : expected, ctx->windowsize);
		}
	}
	ut_assertok(env_set("tftpblocksize", NULL));
	ut_assertok(env_set("tftpwindowsize", "bad"));
	memset(ctx, 0, sizeof(*ctx));
	ret = run_command("tftpboot 2000000 file", 0);
	ut_asserteq(vars ? CMD_RET_FAILURE : CMD_RET_SUCCESS, ret);
	ut_asserteq(vars ? 0 : 1, ctx->requests);
	ut_assertok(env_set("tftpwindowsize", NULL));
	return 0;
}
#endif

static int tftp_test_protected_autostart(struct unit_test_state *uts,
					 struct tftp_test_context *ctx)
{
	char * const argv[] = { "tftpboot", "2000000", "update.bin" };
	int enabled, helper, ret;

	for (enabled = 0; enabled < 2; enabled++) {
		ut_assertok(env_set("autostart", enabled ? "yes" : "no"));
		ut_assertok(env_set(".flags", "autostart:br"));
		for (helper = 0; helper < 3; helper++) {
			memset(ctx, 0, sizeof(*ctx));
			switch (helper) {
			case 0:
				ret = tftpb_run(TFTP_TEST_ADDR, "update.bin");
				break;
			case 1:
				ret = tftpb_run_timeout(TFTP_TEST_ADDR, "update.bin", 100, 0);
				break;
			default:
				ret = tftpb_run_argv(3, argv);
			}
			/* Do not override protection when suppression is required. */
			ut_asserteq(enabled, !!ret);
			ut_asserteq(!enabled, ctx->requests);
			ut_assert(!ctx->autostart);
			ut_asserteq_str(enabled ? "yes" : "no", env_get("autostart"));
		}
		ut_assertok(env_set(".flags", NULL));
	}
	return 0;
}

#if IS_ENABLED(CONFIG_NET_LWIP)
static int tftp_test_addresses(struct unit_test_state *uts,
			       struct tftp_test_context *ctx)
{
	int i, put, ret;

	ut_assertok(env_set("autostart", "no"));
	ut_assertok(env_set("ethrotate", "no"));
	ut_assertok(env_set("ethact", "eth@10003000"));
	ut_assertok(env_set("ipaddr5", "1.1.2.4"));
	ut_assertok(env_set("netmask5", "255.255.255.0"));
	ut_assertok(env_set("bootfile", NULL));
	ut_assertok(env_set("ipaddr", NULL));
	for (put = 0; put <= IS_ENABLED(CONFIG_CMD_TFTPPUT); put++) {
		memset(ctx, 0, sizeof(*ctx));
		ut_assertok(run_command(put ? "tftpput 2000000 20" : "tftpboot 2000000", 0));
		ut_asserteq(1, ctx->requests);
		ut_asserteq_str("01010204.img", ctx->filename);
		ut_asserteq(string_to_ip("1.1.2.4").s_addr, ctx->local_ip.s_addr);
	}
	/* A valid global address must not hide an unset/invalid indexed one. */
	ut_assertok(env_set("ipaddr", "1.1.2.3"));
	for (i = 0; i < 2; i++) {
		ut_assertok(env_set("ipaddr5", i ? "invalid" : NULL));
		memset(ctx, 0, sizeof(*ctx));
		ret = run_command("tftpboot 2000000 explicit.bin", 0);
		ut_asserteq(CMD_RET_FAILURE, ret);
		ut_asserteq(0, ctx->requests);
		ut_asserteq(0, net_boot_file_size);
	}
	ut_assertok(env_set("ethact", "eth@10002000"));
	ut_assertok(env_set("ethrotate", NULL));
	return 0;
}
#endif

static int tftp_test_zero_address(struct unit_test_state *uts,
				  struct tftp_test_context *ctx)
{
	static const char * const commands[] = { "tftpboot 0 file", "tftpboot 0", "tftpboot" };
	void *buffer = map_sysmem(0, sizeof(tftp_test_data));
	u8 saved[sizeof(tftp_test_data)];
	u8 received[sizeof(tftp_test_data)];
	int i, ret;

	ut_assertok(env_set("autostart", "no"));
	ut_assertok(env_set("loadaddr", "0"));
	ut_assertok(env_set("bootfile", "file"));
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		memcpy(saved, buffer, sizeof(saved));
		memset(ctx, 0, sizeof(*ctx));
		ret = run_command(commands[i], 0);
		/* Restore sandbox RAM even if the command failed. */
		memcpy(received, buffer, sizeof(received));
		memcpy(buffer, saved, sizeof(saved));
		ut_assertok(ret);
		ut_asserteq(1, ctx->requests);
		ut_asserteq(0, image_load_addr);
		ut_asserteq(0, env_get_hex("fileaddr", 1));
		ut_asserteq_mem(tftp_test_data, received, sizeof(received));
	}
	if (IS_ENABLED(CONFIG_NET_LWIP) && IS_ENABLED(CONFIG_CMD_TFTPPUT)) {
		memset(ctx, 0, sizeof(*ctx));
		ut_assertok(run_command("tftpput 0 20 file", 0));
		ut_asserteq(1, ctx->requests);
		ut_asserteq(32, net_boot_file_size);
	}
	unmap_sysmem(buffer);
	ut_assertok(env_set("loadaddr", "2000000"));
	image_load_addr = TFTP_TEST_ADDR;
	return 0;
}

static int tftp_test_download(struct unit_test_state *uts,
			      struct tftp_test_context *ctx)
{
	static const char * const autostart[] = { "yes", NULL, "no", "yes-please" };
	const char *timeout, *retries;
	char * const argv[] = { "tftpboot", "2000000", "update.bin" };
	void *buffer = map_sysmem(TFTP_TEST_ADDR, sizeof(tftp_test_data));
	int i, fail, ret;

	ut_assertok(env_set("ethact", "eth@10002000"));
	/* Run the address callbacks used by the legacy stack as well. */
	ut_assertok(run_command("setenv ipaddr 1.1.2.3", 0));
	ut_assertok(run_command("setenv serverip 1.1.2.2", 0));
	ut_assertok(run_command("setenv netmask 255.255.255.0", 0));
	ut_assertok(env_set("tftpserverip", NULL));
	ut_assertok(env_set("netretry", "no"));
	ut_assertok(env_set("tftpsrcp", NULL));
	ut_assertok(env_set("tftpdstp", NULL));
	ut_assertok(tftp_test_probe_arp(uts, ctx));
	ut_assertok(tftp_test_probe_timeout(uts, ctx));

	ut_assertok(tftp_test_protected_autostart(uts, ctx));

#if IS_ENABLED(CONFIG_NET_LWIP)
	ut_assertok(tftp_test_addresses(uts, ctx));
	ut_assertok(tftp_test_options(uts, ctx));
#endif
	ut_assertok(tftp_test_filenames(uts, ctx));
	ut_assertok(tftp_test_zero_address(uts, ctx));

	/* A previous DHCP/download must not override the next command's env. */
	ut_assertok(env_set("autostart", "no"));
	ut_assertok(env_set("loadaddr", "2000000"));
	for (i = 0; i < 3; i++) {
		strcpy(net_boot_file_name, "stale.bin");
		ut_assertok(env_set("bootfile", i == 2 ? NULL : "fresh.bin"));
		memset(ctx, 0, sizeof(*ctx));
		ut_assertok(run_command(i == 1 ? "tftpboot 2000000" : "tftpboot", 0));
		ut_asserteq_str(i == 2 ? "01010203.img" : "fresh.bin", ctx->filename);
		ut_asserteq(1, ctx->requests);
	}
	strcpy(net_boot_file_name, "stale.bin");
	ut_assertok(env_set("bootfile", "helper.bin"));
	ut_assertok(tftpb_run(TFTP_TEST_ADDR, NULL));
	ut_asserteq_str("helper.bin", ctx->filename);

#if IS_ENABLED(CONFIG_NET_LWIP) && IS_ENABLED(CONFIG_NET_TFTP_VARS)
	/* Reject delays outside lwIP's wrap-safe timer range before an RRQ. */
	ut_assertok(env_set("autostart", "no"));
	for (i = 0; i < 3; i++) {
		static const char * const delays[] = {
			"1073741823", "1073741824", "4294967295",
		};

		memset(ctx, 0, sizeof(*ctx));
		net_boot_file_size = 1234;
		ut_assertok(env_set("tftptimeout", delays[i]));
		ret = do_tftpb(NULL, 0, 3, argv);
		ut_asserteq(i ? CMD_RET_FAILURE : CMD_RET_SUCCESS, ret);
		ut_asserteq(i ? 0 : 1, ctx->requests);
		ut_asserteq(i ? 0 : sizeof(tftp_test_data), net_boot_file_size);
		ut_asserteq(-ENODEV, net_lwip_poll());
	}
	ut_assertok(env_set("tftptimeout", NULL));
#endif

#if IS_ENABLED(CONFIG_CMD_ELF_BOOTVX)
	/* Stop before the VxWorks handoff by deliberately omitting bootaddr. */
	ut_assertok(env_set("bootaddr", NULL));
	ut_assertok(env_set("loadaddr", "2000000"));
	ut_assertok(env_set("bootfile", "update.bin"));
	*net_boot_file_name = '\0';
	ut_assertok(env_set("autostart", "yes"));
	for (i = 0; i < 3; i++) {
		memset(ctx, 0, sizeof(*ctx));
		ctx->fail = i == 1;
		ctx->empty = i == 2;
		image_load_addr = TFTP_TEST_ADDR + 4096;
		net_boot_file_size = 1234;
		ut_asserteq(CMD_RET_FAILURE, run_command("bootvx tftp", 0));
		ut_asserteq(1, ctx->requests);
		ut_assert(!ctx->autostart);
		ut_asserteq_str("yes", env_get("autostart"));
		ut_asserteq(i ? 0 : sizeof(tftp_test_data), net_boot_file_size);
		if (!i) {
			ut_asserteq(TFTP_TEST_ADDR, image_load_addr);
			ut_asserteq_mem(tftp_test_data, buffer, sizeof(tftp_test_data));
		}
	}
#endif

	for (i = 0; i < ARRAY_SIZE(autostart); i++) {
		timeout = i & 1 ? NULL : "7000";
		retries = i & 1 ? NULL : "09";
		for (fail = 0; fail < 6; fail++) {
			ut_assertok(env_set("autostart", autostart[i]));
			ut_assertok(env_set("tftptimeout", timeout));
			ut_assertok(env_set("tftptimeoutcountmax", retries));
			memset(buffer, 0, sizeof(tftp_test_data));
			memset(ctx, 0, sizeof(*ctx));
			ctx->fail = fail & 1;

			switch (fail / 2) {
			case 0:
				ret = tftpb_run_timeout(TFTP_TEST_ADDR, "update.bin", 100, 0);
				break;
			case 1:
				ret = tftpb_run(TFTP_TEST_ADDR, "update.bin");
				break;
			default:
				ret = tftpb_run_argv(3, argv) ? -ENOENT : 0;
			}
			ut_asserteq(ctx->fail ? -ENOENT : 0, ret);
			ut_asserteq(1, ctx->requests);
			ut_assert(!ctx->autostart);
			if (!ctx->fail) {
				ut_asserteq(sizeof(tftp_test_data), net_boot_file_size);
				ut_asserteq_mem(tftp_test_data, buffer,
						sizeof(tftp_test_data));
			}
			if (autostart[i])
				ut_asserteq_str(autostart[i], env_get("autostart"));
			else
				ut_assertnull(env_get("autostart"));
			if (timeout) {
				ut_asserteq_str(timeout, env_get("tftptimeout"));
				ut_asserteq_str(retries, env_get("tftptimeoutcountmax"));
			} else {
				ut_assertnull(env_get("tftptimeout"));
				ut_assertnull(env_get("tftptimeoutcountmax"));
			}
		}
	}
	if (IS_ENABLED(CONFIG_CMD_BOOTM)) {
		/* An empty successful download must not boot stale RAM contents. */
		memset(ctx, 0, sizeof(*ctx));
		ctx->empty = true;
		ut_assertok(env_set("autostart", "yes"));
		ut_assertok(do_tftpb(NULL, 0, 3, argv));
		ut_asserteq(1, ctx->requests);
		ut_asserteq(0, net_boot_file_size);

		/*
		 * The same bytes are not bootable. A command must attempt boot
		 * and return its failure, unlike the download-only helpers above.
		 */
		memset(ctx, 0, sizeof(*ctx));
		ut_assertok(env_set("autostart", "yes"));
		ut_assert(do_tftpb(NULL, 0, 3, argv) != CMD_RET_SUCCESS);
		ut_asserteq(1, ctx->requests);
		ut_assert(ctx->autostart);
		ut_asserteq(sizeof(tftp_test_data), net_boot_file_size);
		ut_asserteq_str("yes", env_get("autostart"));
#if IS_ENABLED(CONFIG_NET_LWIP)
		ut_asserteq(-ENODEV, net_lwip_poll());
#endif
	}
#if IS_ENABLED(CONFIG_NET_LWIP) && IS_ENABLED(CONFIG_CMD_TFTPPUT)
	/* Uploading non-bootable bytes is not a request to boot their source. */
	memset(ctx, 0, sizeof(*ctx));
	ut_assertok(env_set("autostart", "yes"));
	ut_assertok(run_command("tftpput 2000000 20 update.bin", 0));
	ut_asserteq(1, ctx->requests);
	ut_assert(ctx->autostart);
	ut_asserteq_str("yes", env_get("autostart"));
	ut_asserteq(-ENODEV, net_lwip_poll());
#endif
	unmap_sysmem(buffer);

	return 0;
}

static int dm_test_tftp_download_only(struct unit_test_state *uts)
{
	static const char * const vars[] = {
		"autostart", "tftptimeout", "tftptimeoutcountmax", "ethact",
		"ipaddr", "serverip", "netmask", "gatewayip", "netretry", "tftpsrcp",
		"tftpdstp", "tftpserverip", "fileaddr", "filesize",
		"bootaddr", "loadaddr", "bootfile", "ethrotate", ".flags",
		"tftpblocksize", "tftpwindowsize", "ipaddr5", "netmask5",
	};
	char saved_bootfile[sizeof(net_boot_file_name)];
	ulong saved_load_addr = image_load_addr;
	char *saved[ARRAY_SIZE(vars)] = {};
	struct tftp_test_context ctx = {};
	const char *value;
	int i, ret = -ENOMEM;

	for (i = 0; i < ARRAY_SIZE(vars); i++) {
		value = env_get(vars[i]);
		if (value) {
			saved[i] = strdup(value);
			if (!saved[i])
				goto out;
		}
	}
	memcpy(saved_bootfile, net_boot_file_name, sizeof(saved_bootfile));

	sandbox_eth_set_tx_handler(0, tftp_test_tx_handler);
	sandbox_eth_set_priv(0, &ctx);
	sandbox_eth_set_tx_handler(1, tftp_test_tx_handler);
	sandbox_eth_set_priv(1, &ctx);
	ret = tftp_test_download(uts, &ctx);
	sandbox_eth_set_tx_handler(0, NULL);
	sandbox_eth_set_priv(0, NULL);
	sandbox_eth_set_tx_handler(1, NULL);
	sandbox_eth_set_priv(1, NULL);
	image_load_addr = saved_load_addr;
	memcpy(net_boot_file_name, saved_bootfile, sizeof(saved_bootfile));
	for (i = 0; i < ARRAY_SIZE(vars); i++) {
		char * const argv[] = { "setenv", (char *)vars[i], saved[i] };

		if (env_do_env_set(0, saved[i] ? 3 : 2, argv, 0))
			ret = -EINVAL;
	}
out:
	for (i = 0; i < ARRAY_SIZE(vars); i++)
		free(saved[i]);

	return ret;
}

DM_TEST(dm_test_tftp_download_only, UTF_SCAN_FDT);
