// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2023 The Android Open Source Project
 */

#include <fastboot.h>
#include <net.h>
#include <net/fastboot_tcp.h>
#include <net/tcp.h>

#define FASTBOOT_TCP_PORT	5554

static const unsigned short handshake_length = 4;
static const uchar *handshake = "FB01";

static char rxbuf[sizeof(u64) + FASTBOOT_COMMAND_LEN + 1];
static char txbuf[sizeof(u64) + FASTBOOT_RESPONSE_LEN + 1];

static u32 data_read;
static u32 tx_last_offs, tx_last_len;

static void tcp_stream_on_rcv_nxt_update(struct tcp_stream *tcp, u32 rx_bytes)
{
	u64	cmd_size;
	__be64	len_be;
	char	saved;
	int	fastboot_command_id, len;

	if (!data_read && rx_bytes >= handshake_length) {
		if (memcmp(rxbuf, handshake, handshake_length)) {
			printf("fastboot: bad handshake\n");
			tcp_stream_close(tcp);
			return;
		}

		tx_last_offs = 0;
		tx_last_len = handshake_length;
		memcpy(txbuf, handshake, handshake_length);

		data_read += handshake_length;
		rx_bytes -= handshake_length;
		if (rx_bytes > 0)
			memmove(rxbuf, rxbuf + handshake_length, rx_bytes);
		return;
	}

	if (rx_bytes < sizeof(u64))
		return;

	memcpy(&cmd_size, rxbuf, sizeof(u64));
	cmd_size = __be64_to_cpu(cmd_size);
	if (rx_bytes < sizeof(u64) + cmd_size)
		return;

	saved = rxbuf[sizeof(u64) + cmd_size];
	rxbuf[sizeof(u64) + cmd_size] = '\0';
	fastboot_command_id = fastboot_handle_command(rxbuf + sizeof(u64),
						      txbuf + sizeof(u64));
	fastboot_handle_boot(fastboot_command_id,
			     strncmp("OKAY", txbuf + sizeof(u64), 4) != 0);
	rxbuf[sizeof(u64) + cmd_size] = saved;

	len = strlen(txbuf + sizeof(u64));
	len_be = __cpu_to_be64(len);
	memcpy(txbuf, &len_be, sizeof(u64));

	tx_last_offs += tx_last_len;
	tx_last_len = len + sizeof(u64);

	data_read += sizeof(u64) + cmd_size;
	rx_bytes -= sizeof(u64) + cmd_size;
	if (rx_bytes > 0)
		memmove(rxbuf, rxbuf + sizeof(u64) + cmd_size, rx_bytes);
}

static int tcp_stream_rx(struct tcp_stream *tcp, u32 rx_offs, void *buf, int len)
{
	memcpy(rxbuf + rx_offs - data_read, buf, len);

	return len;
}

static int tcp_stream_tx(struct tcp_stream *tcp, u32 tx_offs, void *buf, int maxlen)
{
	/* by design: tx_offs >= tx_last_offs */
	if (tx_offs >= tx_last_offs + tx_last_len)
		return 0;

	maxlen = tx_last_offs + tx_last_len - tx_offs;
	memcpy(buf, txbuf + (tx_offs - tx_last_offs), maxlen);

	return maxlen;
}

static int tcp_stream_on_create(struct tcp_stream *tcp)
{
	if (tcp->lport != FASTBOOT_TCP_PORT)
		return 0;

	data_read = 0;
	tx_last_offs = 0;
	tx_last_len = 0;

	tcp->on_rcv_nxt_update = tcp_stream_on_rcv_nxt_update;
	tcp->rx = tcp_stream_rx;
	tcp->tx = tcp_stream_tx;

	return 1;
}

void fastboot_tcp_start_server(void)
{
	memset(net_server_ethaddr, 0, 6);
	tcp_stream_set_on_create_handler(tcp_stream_on_create);

	printf("Using %s device\n", eth_get_name());
	printf("Listening for fastboot command on tcp %pI4\n", &net_ip);
}
