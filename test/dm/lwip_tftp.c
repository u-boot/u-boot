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
	unsigned int opened;
	unsigned int closed;
	unsigned int written;
	unsigned int reads;
	int error;
	int read_error;
	bool initialized;
	bool fail_data_alloc;
	char timeout[4];
};

static struct tftp_protocol_test *tftp_test_active;

static void *tftp_test_open(const char *fname, const char *mode, u8 is_write)
{
	if (!is_write)
		return NULL;
	tftp_test_active->opened++;
	return tftp_test_active;
}

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
	if (test->opcode == 1 || test->opcode == 2) {
		u16 offset = pbuf_memfind(p, "timeout", sizeof("timeout"), IP_UDP_HDR_SIZE);

		memset(test->timeout, 0, sizeof(test->timeout));
		if (offset != 0xffff)
			pbuf_copy_partial(p, test->timeout, sizeof(test->timeout),
					  offset + sizeof("timeout"));
	}
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
	struct tftp_protocol_test *test = handle;

	test->reads++;
	if (test->read_error)
		return test->read_error;
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
	.open = tftp_test_open,
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
	/* Allow the activity timer, then fail the pending DATA allocation. */
	if (test->fail_data_alloc)
		malloc_enable_testing(1);
	pcb->recv(pcb->recv_arg, pcb, p, &test->server, 1234);
	if (test->fail_data_alloc)
		malloc_disable_testing();
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

static int tftp_test_data_alloc_retry(struct unit_test_state *uts,
				      struct tftp_protocol_test *test)
{
	static const char oack[] = "\000\006blksize\0001024";
	u8 ack[] = { 0, 4, 0, 0 };
	unsigned int sent;
	int i;

	for (i = 0; i < 2; i++) {
		ut_assertok(tftp_test_start(test, true));
		ut_assertok(tftp_set_timeout(30, 3));
		ack[3] = 0;
		test->fail_data_alloc = true;
		if (i)
			ut_assertok(tftp_test_input(test, oack, sizeof(oack), 0));
		else
			ut_assertok(tftp_test_input(test, ack, sizeof(ack), 0));
		test->fail_data_alloc = false;
		ut_asserteq(1, test->sent);
		ut_asserteq(0, test->reads);

		/* An old ACK must not reject or postpone the pending block. */
		ut_assertok(tftp_test_input(test, ack, sizeof(ack), 0));
		ut_asserteq(1, test->sent);
		timer_test_add_offset(40);
		sys_check_timeouts();
		ut_asserteq(2, test->sent);
		ut_asserteq(3, test->opcode);
		ut_asserteq(1, test->block);
		ut_asserteq(1, test->reads);

		/* Retrying the constructed packet must not consume input again. */
		timer_test_add_offset(40);
		sys_check_timeouts();
		ut_asserteq(3, test->sent);
		ut_asserteq(1, test->block);
		ut_asserteq(1, test->reads);

		/* Repeat after freeing an acknowledged, nonempty DATA buffer. */
		ack[3] = 1;
		test->fail_data_alloc = true;
		ut_assertok(tftp_test_input(test, ack, sizeof(ack), 0));
		test->fail_data_alloc = false;
		ut_asserteq(3, test->sent);
		ut_asserteq(1, test->reads);
		ut_assertok(tftp_test_input(test, ack, sizeof(ack), 0));
		ut_asserteq(3, test->sent);
		ack[3] = 0;
		ut_assertok(tftp_test_input(test, ack, sizeof(ack), 0));
		ut_asserteq(3, test->sent);
		if (i) {
			ut_assertok(tftp_test_input(test, oack, sizeof(oack), 0));
			ut_asserteq(3, test->sent);
		}
		ut_asserteq(1, test->reads);
		timer_test_add_offset(40);
		sys_check_timeouts();
		ut_asserteq(4, test->sent);
		ut_asserteq(3, test->opcode);
		ut_asserteq(2, test->block);
		ut_asserteq(2, test->reads);
		ut_asserteq(0, test->error);
	}

	/* A repeated OACK can also retry constructing the first DATA packet. */
	ut_assertok(tftp_test_start(test, true));
	ut_assertok(tftp_set_timeout(30, 1));
	test->fail_data_alloc = true;
	ut_assertok(tftp_test_input(test, oack, sizeof(oack), 0));
	test->fail_data_alloc = false;
	ut_asserteq(0, test->reads);
	timer_test_add_offset(20);
	ut_assertok(tftp_test_input(test, oack, sizeof(oack), 0));
	ut_asserteq(2, test->sent);
	ut_asserteq(3, test->opcode);
	ut_asserteq(1, test->block);
	ut_asserteq(1, test->reads);
	timer_test_add_offset(20);
	sys_check_timeouts();
	ut_asserteq(3, test->sent);
	ut_asserteq(1, test->reads);

	/* A read error during timer recovery must close without rearming it. */
	ut_assertok(tftp_test_start(test, true));
	ut_assertok(tftp_set_timeout(30, 1));
	ack[3] = 0;
	test->fail_data_alloc = true;
	ut_assertok(tftp_test_input(test, ack, sizeof(ack), 0));
	test->fail_data_alloc = false;
	test->read_error = -EIO;
	timer_test_add_offset(40);
	sys_check_timeouts();
	ut_asserteq(2, test->error);
	ut_asserteq(1, test->closed);
	ut_asserteq(1, test->reads);
	sent = test->sent;
	timer_test_add_offset(40);
	sys_check_timeouts();
	ut_asserteq(sent, test->sent);
	ut_asserteq(1, test->closed);
	return 0;
}

static int tftp_test_duplicate_ack(struct unit_test_state *uts,
				   struct tftp_protocol_test *test)
{
	u8 ack[] = { 0, 4, 0, 0 };
	unsigned int block, i, sent;

	for (block = 1; block <= 3; block += 2) {
		ut_assertok(tftp_test_start(test, true));
		ut_assertok(tftp_set_timeout(30, 1));
		for (i = 0; i < block; i++) {
			ack[3] = i;
			ut_assertok(tftp_test_input(test, ack, sizeof(ack), 0));
		}
		ut_asserteq(3, test->opcode);
		ut_asserteq(block, test->block);
		sent = test->sent;
		timer_test_add_offset(20);
		/* Ignore all older ACKs, not just the immediately preceding one. */
		for (i = 0; i < block; i++) {
			ack[3] = i;
			ut_assertok(tftp_test_input(test, ack, sizeof(ack), 0));
			ut_asserteq(sent, test->sent);
		}
		/* An ACK for an unsent block must not advance the input either. */
		ack[3] = block + 1;
		ut_assertok(tftp_test_input(test, ack, sizeof(ack), 0));
		ut_asserteq(sent, test->sent);
		ut_asserteq(block, test->reads);
		timer_test_add_offset(20);
		sys_check_timeouts();
		ut_asserteq(sent + 1, test->sent);
		ut_asserteq(block, test->block);
		ut_asserteq(block, test->reads);
		ack[3] = 0;
		ut_assertok(tftp_test_input(test, ack, sizeof(ack), 0));
		timer_test_add_offset(40);
		sys_check_timeouts();
		ut_asserteq(-1, test->error);
		ut_asserteq(1, test->closed);
		ut_asserteq(sent + 1, test->sent);
	}
	return 0;
}

static int tftp_test_duplicate_oack(struct unit_test_state *uts,
				    struct tftp_protocol_test *test)
{
	static const char oack[] = "\000\006blksize\0001024";
	unsigned int sent;
	int put;

	for (put = 0; put < 2; put++) {
		ut_assertok(tftp_test_start(test, put));
		ut_assertok(tftp_set_timeout(30, 1));
		ut_assertok(tftp_test_input(test, oack, sizeof(oack), 0));
		sent = test->sent;
		timer_test_add_offset(20);
		ut_assertok(tftp_test_input(test, oack, sizeof(oack), 0));
		ut_asserteq(sent + 1, test->sent);
		ut_asserteq(put ? 3 : 4, test->opcode);
		ut_asserteq(put ? 1 : 0, test->block);

		/* A duplicate neither postpones a retry nor replenishes its budget. */
		timer_test_add_offset(20);
		sys_check_timeouts();
		ut_asserteq(sent + 2, test->sent);
		ut_assertok(tftp_test_input(test, oack, sizeof(oack), 0));
		timer_test_add_offset(40);
		sys_check_timeouts();
		ut_asserteq(-1, test->error);
		ut_asserteq(1, test->closed);
		ut_asserteq(sent + 3, test->sent);
	}
	return 0;
}

static int tftp_test_stale_control(struct unit_test_state *uts,
				   struct tftp_protocol_test *test)
{
	static const char oack[] = "\000\006blksize\00016";
	static const char changed[] = "\000\006blksize\0008";
	static const char invalid[] = "\000\006unknown\0001";
	u8 data[4 + 16] = { 0, 3 };
	u8 ack[] = { 0, 4, 0, 0 };
	unsigned int end, i, sent, reads, written;
	int put, wrap;

	for (put = 0; put < 2; put++) {
		for (wrap = 0; wrap < 2; wrap++) {
			ut_assertok(tftp_test_start(test, put));
			ut_assertok(tftp_set_timeout(30, 1));
			ut_assertok(tftp_test_input(test, oack, sizeof(oack), 0));
			end = wrap ? 65536 : 1;
			for (i = 1; i <= end; i++) {
				put_unaligned_be16(i, (put ? ack : data) + 2);
				ut_assertok(tftp_test_input(test, put ? ack : data,
							    put ? sizeof(ack) : sizeof(data), 0));
			}
			reads = put ? end + 1 : 0;
			written = put ? 0 : end * 16;
			ut_asserteq(reads, test->reads);
			ut_asserteq(written, test->written);
			ut_asserteq(put ? 3 : 4, test->opcode);
			ut_asserteq((u16)(end + put), test->block);
			sent = test->sent;
			timer_test_add_offset(20);
			if (put) {
				/* Two blocks old, including ACK 65535 while awaiting ACK 1. */
				put_unaligned_be16(end - 1, ack + 2);
				ut_assertok(tftp_test_input(test, ack, sizeof(ack), 0));
				ut_asserteq(sent, test->sent);
			}
			/*
			 * A matching OACK must not restart negotiation after DATA,
			 * even when the block number has wrapped back to one.
			 */
			ut_assertok(tftp_test_input(test, oack, sizeof(oack), 5));
			ut_asserteq(sent, test->sent);
			ut_asserteq(reads, test->reads);
			ut_asserteq(written, test->written);
			ut_asserteq(0, test->error);
			ut_asserteq(0, test->closed);
			timer_test_add_offset(20);
			sys_check_timeouts();
			ut_asserteq(sent + 1, test->sent);
			ut_asserteq(put ? 3 : 4, test->opcode);
			ut_asserteq((u16)(end + put), test->block);
			ut_assertok(tftp_test_input(test, oack, sizeof(oack), 0));
			timer_test_add_offset(40);
			sys_check_timeouts();
			ut_asserteq(-1, test->error);
			ut_asserteq(1, test->closed);
			ut_asserteq(sent + 1, test->sent);
			ut_asserteq(reads, test->reads);
			ut_asserteq(written, test->written);
		}

		/* Only matching, valid options may be ignored after progress. */
		ut_assertok(tftp_test_start(test, put));
		ut_assertok(tftp_test_input(test, oack, sizeof(oack), 0));
		put_unaligned_be16(1, (put ? ack : data) + 2);
		ut_assertok(tftp_test_input(test, put ? ack : data,
					    put ? sizeof(ack) : sizeof(data), 0));
		ut_assertok(tftp_test_input(test, changed, sizeof(changed), 0));
		ut_asserteq(5, test->opcode);
		ut_asserteq(4, test->block);
		ut_assertok(tftp_test_input(test, invalid, sizeof(invalid), 0));
		ut_asserteq(5, test->opcode);
		ut_asserteq(8, test->block);
		ut_asserteq(8, test->error);
		ut_asserteq(1, test->closed);
	}
	return 0;
}

static int tftp_test_timeout_range(struct unit_test_state *uts,
				   struct tftp_protocol_test *test)
{
	ut_assertok(tftp_test_start(test, false));
	ut_asserteq(ERR_ARG, tftp_set_timeout(0, 1));
	ut_assertok(tftp_set_timeout(1, 1));
	ut_assertok(tftp_set_timeout(U32_MAX / 4, 1));
	ut_asserteq(ERR_ARG, tftp_set_timeout(U32_MAX / 4 + 1, 1));
	ut_asserteq(ERR_ARG, tftp_set_timeout(U32_MAX, 1));
	return 0;
}

static int tftp_test_timeout_option(struct unit_test_state *uts,
				    struct tftp_protocol_test *test)
{
	static const u32 delays[] = { 999, 1000, 1500, 255000, 256000 };
	static const char * const values[] = { "", "1", "1", "255", "" };
	static const char oack[] = "\000\006timeout\0001";
	static const char bad[] = "\000\006timeout\0002";
	static const char maximum[] = "\000\006timeout\000255";
	static const char duplicate[] = "\000\006timeout\0001\000TIMEOUT\0001";
	static const u8 ack[] = { 0, 4, 0, 0 };
	int i, put;

	for (put = 0; put < 2; put++) {
		for (i = 0; i < ARRAY_SIZE(delays); i++) {
			if (test->initialized)
				tftp_cleanup();
			memset(test, 0, sizeof(*test));
			ipaddr_aton("1.1.2.2", &test->server);
			ut_assertok(tftp_init_client(&tftp_test_callbacks));
			test->initialized = true;
			tftp_client_set_blksize(1024);
			ut_assertok(tftp_set_timeout(delays[i], 1));
			if (put)
				ut_assertok(tftp_put(test, &test->server, 69, "file",
						     TFTP_MODE_OCTET));
			else
				ut_assertok(tftp_get(test, &test->server, 69, "file",
						     TFTP_MODE_OCTET));
			ut_asserteq_str(values[i], test->timeout);
			if (i == 1 || i == 2) {
				ut_assertok(tftp_test_input(test, oack, sizeof(oack), 5));
				ut_asserteq(0, test->error);
				ut_asserteq(put ? 3 : 4, test->opcode);
				ut_assertok(tftp_test_input(test, oack, sizeof(oack), 0));
				ut_asserteq(0, test->error);
				ut_asserteq(put ? 3 : 4, test->opcode);
				if (i == 2) {
					unsigned int sent = test->sent;

					timer_test_add_offset(1001);
					sys_check_timeouts();
					ut_asserteq(sent, test->sent);
					timer_test_add_offset(500);
					sys_check_timeouts();
					ut_asserteq(sent + 1, test->sent);
				}
				ut_assertok(tftp_test_input(test, duplicate, sizeof(duplicate), 0));
			} else {
				if (i == 3) {
					ut_assertok(tftp_test_input(test, maximum,
								    sizeof(maximum), 0));
					ut_asserteq(0, test->error);
					ut_asserteq(put ? 3 : 4, test->opcode);
				}
				/* Reject unrequested or non-echoed timeout values. */
				ut_assertok(tftp_test_input(test, bad, sizeof(bad), 0));
			}
			ut_asserteq(8, test->error);
			ut_asserteq(1, test->closed);
		}
	}
	/* An ACK 0 still works when the server ignores all WRQ options. */
	ut_assertok(tftp_test_start(test, true));
	ut_assertok(tftp_test_input(test, ack, sizeof(ack), 0));
	ut_asserteq(3, test->opcode);
	ut_asserteq(1, test->block);
	ut_asserteq(0, test->error);
	return 0;
}

static int tftp_test_server_retries(struct unit_test_state *uts,
				    struct tftp_protocol_test *test)
{
	static const char wrq[] = "\000\002file\000octet";
	static const char other[] = "\000\002other\000octet";
	u8 data[4 + 512] = { 0, 3, 0, 1 };
	unsigned int sent;

	if (test->initialized)
		tftp_cleanup();
	memset(test, 0, sizeof(*test));
	ipaddr_aton("1.1.2.2", &test->server);
	ut_assertok(tftp_init_server(&tftp_test_callbacks));
	test->initialized = true;
	test->port = 69;
	ut_assertok(tftp_set_timeout(30, 1));
	ut_assertok(tftp_test_input(test, wrq, sizeof(wrq), 0));
	ut_asserteq(1, test->opened);
	ut_asserteq(4, test->opcode);
	ut_asserteq(0, test->block);
	sent = test->sent;

	timer_test_add_offset(20);
	ut_assertok(tftp_test_input(test, wrq, sizeof(wrq), 5));
	ut_asserteq(sent + 1, test->sent);
	ut_asserteq(4, test->opcode);
	ut_asserteq(0, test->block);
	ut_asserteq(1, test->opened);
	ut_assertok(tftp_test_input(test, other, sizeof(other), 0));
	ut_asserteq(5, test->opcode);
	ut_asserteq(1, test->opened);

	/* The repeated WRQ did not postpone the first retry or reset its budget. */
	timer_test_add_offset(20);
	sys_check_timeouts();
	ut_asserteq(sent + 3, test->sent);
	ut_asserteq(4, test->opcode);
	ut_asserteq(0, test->block);
	ut_assertok(tftp_test_input(test, wrq, sizeof(wrq), 0));
	timer_test_add_offset(40);
	sys_check_timeouts();
	ut_asserteq(-1, test->error);
	ut_asserteq(1, test->closed);
	ut_asserteq(1, test->opened);

	/* A later transfer still works, but a WRQ cannot rewind received data. */
	ut_assertok(tftp_test_input(test, wrq, sizeof(wrq), 0));
	ut_asserteq(2, test->opened);
	ut_assertok(tftp_test_input(test, data, sizeof(data), 0));
	ut_asserteq(512, test->written);
	ut_assertok(tftp_test_input(test, wrq, sizeof(wrq), 0));
	ut_asserteq(5, test->opcode);
	ut_asserteq(2, test->opened);
	put_unaligned_be16(2, data + 2);
	ut_assertok(tftp_test_input(test, data, 4, 0));
	ut_asserteq(2, test->closed);
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
	if (!ret)
		ret = tftp_test_data_alloc_retry(uts, &test);
	if (!ret)
		ret = tftp_test_duplicate_ack(uts, &test);
	if (!ret)
		ret = tftp_test_duplicate_oack(uts, &test);
	if (!ret)
		ret = tftp_test_stale_control(uts, &test);
	if (!ret)
		ret = tftp_test_timeout_range(uts, &test);
	if (!ret)
		ret = tftp_test_timeout_option(uts, &test);
	if (!ret)
		ret = tftp_test_server_retries(uts, &test);
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
