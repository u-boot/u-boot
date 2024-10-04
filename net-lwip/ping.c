// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <console.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <lwip/icmp.h>
#include <lwip/inet_chksum.h>
#include <lwip/raw.h>
#include <lwip/timeouts.h>
#include <net-lwip.h>
#include <time.h>

#define PING_DELAY_MS 1000
#define PING_TIMEOUT_MS 10000
/* Ping identifier - must fit on a u16_t */
#define PING_ID 0xAFAF

static const ip_addr_t *ping_target;
static struct raw_pcb *ping_pcb;
static uint16_t ping_seq_num;

static u8_t ping_recv(void *arg, struct raw_pcb *pcb, struct pbuf *p,
		      const ip_addr_t *addr)
{
	struct icmp_echo_hdr *iecho;
	bool *alive = arg;

	if (addr->addr != ping_target->addr)
		return 0;

	if ((p->tot_len >= (IP_HLEN + sizeof(struct icmp_echo_hdr))) &&
	    pbuf_remove_header(p, IP_HLEN) == 0) {
		iecho = (struct icmp_echo_hdr *)p->payload;

		if ((iecho->id == PING_ID) &&
		    (iecho->seqno == lwip_htons(ping_seq_num))) {
			*alive = true;
			printf("host %s is alive\n", ipaddr_ntoa(addr));
			pbuf_free(p);
			return 1; /* eat the packet */
		}
		/* not eaten, restore original packet */
		pbuf_add_header(p, IP_HLEN);
	}

	return 0; /* don't eat the packet */
}

static int ping_raw_init(void *recv_arg)
{
	ping_pcb = raw_new(IP_PROTO_ICMP);
	if (!ping_pcb)
		return -ENOMEM;

	raw_recv(ping_pcb, ping_recv, recv_arg);
	raw_bind(ping_pcb, IP_ADDR_ANY);

	return 0;
}

static void ping_raw_stop(void)
{
	if (ping_pcb != NULL) {
		raw_remove(ping_pcb);
		ping_pcb = NULL;
	}
}

static void ping_prepare_echo(struct icmp_echo_hdr *iecho)
{
	ICMPH_TYPE_SET(iecho, ICMP_ECHO);
	ICMPH_CODE_SET(iecho, 0);
	iecho->chksum = 0;
	iecho->id = PING_ID;
	iecho->seqno = lwip_htons(++ping_seq_num);

	iecho->chksum = inet_chksum(iecho, sizeof(*iecho));
}

static void ping_send_icmp(struct raw_pcb *raw, const ip_addr_t *addr)
{
	struct pbuf *p;
	struct icmp_echo_hdr *iecho;
	size_t ping_size = sizeof(struct icmp_echo_hdr);

	p = pbuf_alloc(PBUF_IP, (u16_t)ping_size, PBUF_RAM);
	if (!p)
		return;

	if ((p->len == p->tot_len) && (p->next == NULL)) {
		iecho = (struct icmp_echo_hdr *)p->payload;
		ping_prepare_echo(iecho);
		raw_sendto(raw, p, addr);
	}

	pbuf_free(p);
}

static void ping_send(void *arg)
{
	struct raw_pcb *pcb = (struct raw_pcb *)arg;

	ping_send_icmp(pcb, ping_target);
	sys_timeout(PING_DELAY_MS, ping_send, ping_pcb);
}

static int ping_loop(const ip_addr_t* addr)
{
	bool alive;
	ulong start;
	int ret;

	printf("Using %s device\n", eth_get_name());

	ret = ping_raw_init(&alive);
	if (ret < 0)
		return ret;
	ping_target = addr;
	ping_seq_num = 0;

	start = get_timer(0);
	ping_send(ping_pcb);

	do {
		eth_rx();
		if (alive)
			break;
		sys_check_timeouts();
		if (ctrlc()) {
			printf("\nAbort\n");
			break;
		}
	} while (get_timer(start) < PING_TIMEOUT_MS);

	sys_untimeout(ping_send, ping_pcb);
	ping_raw_stop();
	ping_target = NULL;

	if (alive) {
		alive = false;
		return 0;
	}
	printf("ping failed; host %s is not alive\n", ipaddr_ntoa(addr));
	return -1;
}

int do_ping(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ip_addr_t addr;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (!ipaddr_aton(argv[1], &addr))
		return CMD_RET_USAGE;

	if (ping_loop(&addr) < 0)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}
