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
};

/* Answer an RRQ with one short DATA block or a file-not-found error. */
static int tftp_test_tx_handler(struct udevice *dev, void *packet,
				unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct tftp_test_context *ctx = priv->priv;
	struct ethernet_hdr *eth = packet, *reply_eth;
	struct ip_udp_hdr *ip, *reply_ip;
	const char *data = ctx->fail ? "not found" : tftp_test_data;
	unsigned int data_len = strlen(data) + 1;
	unsigned int reply_len, header_len = 4;
	u16 opcode, reply_opcode = ctx->fail ? 5 : 3, block = 1;
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
	} else if (opcode == 1 && ntohs(ip->udp_dst) == 69) {
		unsigned int i, request_len = len - ETHER_HDR_SIZE - IP_UDP_HDR_SIZE;

		if (!ctx->requests++)
			ctx->request_start = get_timer(0);
		payload = (void *)(ip + 1);
		for (i = 2; i + 10 <= request_len; i++) {
			if (!memcmp(payload + i, "timeout\0", 8))
				ctx->advertised_timeout = dectoul((char *)payload + i + 8, NULL);
		}
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
	} else {
		return 0;
	}
	if (priv->recv_packets >= PKTBUFSRX)
		return -ENOSPC;

	reply_len = ETHER_HDR_SIZE + IP_UDP_HDR_SIZE + header_len + data_len;
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
	put_unaligned_be16(reply_opcode, payload);
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

static int tftp_test_download(struct unit_test_state *uts,
			      struct tftp_test_context *ctx)
{
	static const char * const autostart[] = { "yes", NULL, "no", "yes-please" };
	const char *timeout, *retries;
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

	for (i = 0; i < ARRAY_SIZE(autostart); i++) {
		timeout = i & 1 ? NULL : "7000";
		retries = i & 1 ? NULL : "09";
		for (fail = 0; fail < 2; fail++) {
			ut_assertok(env_set("autostart", autostart[i]));
			ut_assertok(env_set("tftptimeout", timeout));
			ut_assertok(env_set("tftptimeoutcountmax", retries));
			memset(buffer, 0, sizeof(tftp_test_data));
			memset(ctx, 0, sizeof(*ctx));
			ctx->fail = fail;

			ret = tftpb_run_timeout(TFTP_TEST_ADDR, "update.bin", 100, 0);
			ut_asserteq(fail ? -ENOENT : 0, ret);
			ut_asserteq(1, ctx->requests);
			ut_assert(!ctx->autostart);
			if (!fail) {
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
	unmap_sysmem(buffer);

	return 0;
}

static int dm_test_tftp_download_only(struct unit_test_state *uts)
{
	static const char * const vars[] = {
		"autostart", "tftptimeout", "tftptimeoutcountmax", "ethact",
		"ipaddr", "serverip", "netmask", "gatewayip", "netretry", "tftpsrcp",
		"tftpdstp", "tftpserverip", "fileaddr", "filesize",
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
	ret = tftp_test_download(uts, &ctx);
	sandbox_eth_set_tx_handler(0, NULL);
	sandbox_eth_set_priv(0, NULL);
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
