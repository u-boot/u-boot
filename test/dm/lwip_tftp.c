// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2026 James Hilliard <james.hilliard1@gmail.com> */

#include <dm.h>
#include <env.h>
#include <malloc.h>
#include <net.h>
#include <time.h>
#include <asm/unaligned.h>
#include <dm/test.h>
#include <lwip/apps/tftp_client.h>
#include <lwip/apps/tftp_server.h>
#include <lwip/timeouts.h>
#include <lwip/udp.h>
#include <test/ut.h>

struct tftp_protocol_test {
	ip_addr_t server;
	u16 port;
	u16 opcode;
	u16 block;
	unsigned int sent;
	unsigned int closed;
	unsigned int written;
	int error;
	bool initialized;
};

static struct tftp_protocol_test *tftp_test_active;

/* Capture IP output without depending on ARP or a real TFTP server. */
static err_t tftp_test_output(struct netif *netif, struct pbuf *p,
			      const ip4_addr_t *addr)
{
	struct tftp_protocol_test *test = tftp_test_active;
	u8 packet[IP_UDP_HDR_SIZE + 4];

	pbuf_copy_partial(p, packet, sizeof(packet), 0);
	test->port = get_unaligned_be16(packet + IP_HDR_SIZE);
	test->opcode = get_unaligned_be16(packet + IP_UDP_HDR_SIZE);
	test->block = get_unaligned_be16(packet + IP_UDP_HDR_SIZE + 2);
	test->sent++;
	return ERR_OK;
}

static void tftp_test_close(void *handle)
{
	struct tftp_protocol_test *test = handle;

	test->closed++;
}

static int tftp_test_read(void *handle, void *buf, int len)
{
	memset(buf, 0x5a, len);
	return len;
}

static int tftp_test_write(void *handle, struct pbuf *p)
{
	struct tftp_protocol_test *test = handle;

	test->written += p->tot_len;
	return 0;
}

static void tftp_test_error(void *handle, int err, const char *msg, int len)
{
	struct tftp_protocol_test *test = handle;

	test->error = err;
}

static const struct tftp_context tftp_test_callbacks = {
	.close = tftp_test_close,
	.read = tftp_test_read,
	.write = tftp_test_write,
	.error = tftp_test_error,
};

/* Deliver a UDP payload through the callback registered by the real client. */
static int tftp_test_input(struct tftp_protocol_test *test, const void *data,
			   unsigned int len, unsigned int split)
{
	struct udp_pcb *pcb;
	struct pbuf *p, *tail;

	for (pcb = udp_pcbs; pcb; pcb = pcb->next)
		if (pcb->local_port == test->port)
			break;
	if (!pcb || !pcb->recv)
		return -ENOENT;
	if (!split)
		split = len;
	p = pbuf_alloc(PBUF_RAW, split, PBUF_RAM);
	if (!p)
		return -ENOMEM;
	pbuf_take(p, data, split);
	if (split < len) {
		tail = pbuf_alloc(PBUF_RAW, len - split, PBUF_RAM);
		if (!tail) {
			pbuf_free(p);
			return -ENOMEM;
		}
		pbuf_take(tail, data + split, len - split);
		pbuf_cat(p, tail);
	}
	pcb->recv(pcb->recv_arg, pcb, p, &test->server, 1234);
	return 0;
}

static int tftp_test_start(struct tftp_protocol_test *test, bool put)
{
	err_t err;

	if (test->initialized)
		tftp_cleanup();
	memset(test, 0, sizeof(*test));
	ipaddr_aton("1.1.2.2", &test->server);
	err = tftp_init_client(&tftp_test_callbacks);
	if (err)
		return err;
	test->initialized = true;
	tftp_client_set_blksize(1024);
	if (put)
		return tftp_put(test, &test->server, 69, "file", TFTP_MODE_OCTET);
	return tftp_get(test, &test->server, 69, "file", TFTP_MODE_OCTET);
}

static int tftp_test_oack(struct unit_test_state *uts,
			  struct tftp_protocol_test *test)
{
	static const char * const invalid[] = {
		"", "blksize", "blksize\000", "blksize\0000\000",
		"blksize\0001025\000", "blksize\000-8\000",
		"blksize\0004294967296\000", "unknown\0001\000",
		"blksize\0001024\000blksize\0001024\000",
		"blksize\0001024\000tsize", "tsize\0001\000junk\0002\000",
	};
	static const unsigned int lengths[] = {
		0, 7, 8, 10, 13, 11, 19, 10, 26, 18, 15,
	};
	static const char valid[] = "\000\006BLKSIZE\0001024\000TSIZE\0007";
	static const u8 data[] = { 0, 3, 0, 1, 'a', 'b', 'c' };
	u8 packet[64] = { 0, 6 };
	int i, split;

	for (i = 0; i < ARRAY_SIZE(invalid); i++) {
		ut_assertok(tftp_test_start(test, false));
		memcpy(packet + 2, invalid[i], lengths[i]);
		ut_assertok(tftp_test_input(test, packet, lengths[i] + 2, 0));
		ut_asserteq(8, test->error);
		ut_asserteq(1, test->closed);
		ut_asserteq(5, test->opcode);
	}
	for (split = 0; split < 2; split++) {
		ut_assertok(tftp_test_start(test, false));
		ut_assertok(tftp_test_input(test, valid, sizeof(valid), split ? 5 : 0));
		ut_asserteq(0, test->error);
		ut_asserteq(4, test->opcode);
		ut_asserteq(0, test->block);
		ut_asserteq(7, tftp_client_get_tsize());
		ut_assertok(tftp_test_input(test, data, sizeof(data), 0));
		ut_asserteq(3, test->written);
		ut_asserteq(1, test->closed);
	}
	/* A write request without tsize must reject an unsolicited tsize. */
	ut_assertok(tftp_test_start(test, true));
	ut_assertok(tftp_test_input(test, valid, sizeof(valid), 0));
	ut_asserteq(8, test->error);
	return 0;
}

static int dm_test_lwip_tftp_protocol(struct unit_test_state *uts)
{
	static const char * const vars[] = { "ethact", "ipaddr", "netmask" };
	struct tftp_protocol_test test = {};
	struct net_lwip_ctx net = {};
	char *saved[ARRAY_SIZE(vars)] = {};
	netif_output_fn output;
	int i, ret = -ENOMEM;

	for (i = 0; i < ARRAY_SIZE(vars); i++) {
		const char *value = env_get(vars[i]);

		if (value) {
			saved[i] = strdup(value);
			if (!saved[i])
				goto free_env;
		}
	}
	if (env_set("ethact", "eth@10002000") || env_set("ipaddr", "1.1.2.3") ||
	    env_set("netmask", "255.255.255.0"))
		goto restore_env;
	ret = net_lwip_start(&net, NET_LWIP_ADDR_ENV_STRICT);
	if (ret)
		goto restore_env;
	output = net.netif->output;
	net.netif->output = tftp_test_output;
	tftp_test_active = &test;
	ret = tftp_test_oack(uts, &test);
	if (test.initialized)
		tftp_cleanup();
	net.netif->output = output;
	tftp_test_active = NULL;
	net_lwip_stop(&net);
restore_env:
	for (i = 0; i < ARRAY_SIZE(vars); i++)
		if (env_set(vars[i], saved[i]))
			ret = -EINVAL;
free_env:
	for (i = 0; i < ARRAY_SIZE(vars); i++)
		free(saved[i]);
	return ret;
}
DM_TEST(dm_test_lwip_tftp_protocol, UTF_SCAN_FDT);
