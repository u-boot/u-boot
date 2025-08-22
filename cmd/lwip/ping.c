// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <console.h>
#include <dm/device.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <lwip/icmp.h>
#include <lwip/inet_chksum.h>
#include <lwip/raw.h>
#include <lwip/timeouts.h>
#include <net.h>
#include <time.h>

U_BOOT_CMD(ping, 2, 1, do_ping, "send ICMP ECHO_REQUEST to network host",
	   "pingAddressOrHostName");

#define PING_DELAY_MS 1000
#define PING_COUNT 5
/* Ping identifier - must fit on a u16_t */
#define PING_ID 0xAFAF

struct ping_ctx {
	ip_addr_t target;
	struct raw_pcb *pcb;
	struct icmp_echo_hdr *iecho;
	uint16_t seq_num;
	bool alive;
};

static u8_t ping_recv(void *arg, struct raw_pcb *pcb, struct pbuf *p,
		      const ip_addr_t *addr)
{
	struct ping_ctx *ctx = arg;
	struct icmp_echo_hdr *iecho = ctx->iecho;

	if (addr->addr != ctx->target.addr)
		return 0;

	if ((p->tot_len >= (IP_HLEN + sizeof(struct icmp_echo_hdr))) &&
	    pbuf_remove_header(p, IP_HLEN) == 0) {
		iecho = (struct icmp_echo_hdr *)p->payload;

		if (iecho->id == PING_ID &&
		    iecho->seqno == lwip_htons(ctx->seq_num)) {
			ctx->alive = true;
			printf("host %s is alive\n", ipaddr_ntoa(addr));
			pbuf_free(p);
			return 1; /* eat the packet */
		}
		/* not eaten, restore original packet */
		pbuf_add_header(p, IP_HLEN);
	}

	return 0; /* don't eat the packet */
}

static int ping_raw_init(struct ping_ctx *ctx)
{
	ctx->pcb = raw_new(IP_PROTO_ICMP);
	if (!ctx->pcb)
		return -ENOMEM;

	raw_recv(ctx->pcb, ping_recv, ctx);
	raw_bind(ctx->pcb, IP_ADDR_ANY);

	return 0;
}

static void ping_raw_stop(struct ping_ctx *ctx)
{
	if (ctx->pcb)
		raw_remove(ctx->pcb);
}

static void ping_prepare_echo(struct ping_ctx *ctx)
{
	struct icmp_echo_hdr *iecho = ctx->iecho;

	ICMPH_TYPE_SET(iecho, ICMP_ECHO);
	ICMPH_CODE_SET(iecho, 0);
	iecho->chksum = 0;
	iecho->id = PING_ID;
	iecho->seqno = lwip_htons(ctx->seq_num);

	iecho->chksum = inet_chksum(iecho, sizeof(*iecho));
}

static void ping_send_icmp(struct ping_ctx *ctx)
{
	struct pbuf *p;
	size_t ping_size = sizeof(struct icmp_echo_hdr);

	p = pbuf_alloc(PBUF_IP, (u16_t)ping_size, PBUF_RAM);
	if (!p)
		return;

	if (p->len == p->tot_len && !p->next) {
		ctx->iecho = (struct icmp_echo_hdr *)p->payload;
		ping_prepare_echo(ctx);
		raw_sendto(ctx->pcb, p, &ctx->target);
	}

	pbuf_free(p);
}

static void ping_send(void *arg)
{
	struct ping_ctx *ctx = arg;

	ctx->seq_num++;
	if (ctx->seq_num <= PING_COUNT) {
		ping_send_icmp(ctx);
		sys_timeout(PING_DELAY_MS, ping_send, ctx);
	}
}

static int ping_loop(struct udevice *udev, const ip_addr_t *addr)
{
	struct ping_ctx ctx = {};
	struct netif *netif;
	int ret;

	netif = net_lwip_new_netif(udev);
	if (!netif)
		return -ENODEV;

	printf("Using %s device\n", udev->name);

	ret = ping_raw_init(&ctx);
	if (ret < 0) {
		net_lwip_remove_netif(netif);
		return ret;
	}

	ctx.target = *addr;

	ping_send(&ctx);

	do {
		net_lwip_rx(udev, netif);
		if (ctx.alive)
			break;
		if (ctrlc()) {
			printf("\nAbort\n");
			break;
		}
	} while (ctx.seq_num <= PING_COUNT);

	sys_untimeout(ping_send, &ctx);
	ping_raw_stop(&ctx);

	net_lwip_remove_netif(netif);

	if (ctx.alive)
		return 0;

	printf("ping failed; host %s is not alive\n", ipaddr_ntoa(addr));
	return -1;
}

int do_ping(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ip_addr_t addr;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (net_lwip_dns_resolve(argv[1], &addr))
		return CMD_RET_USAGE;

	net_try_count = 1;
restart:
	if (net_lwip_eth_start() < 0 || ping_loop(eth_get_dev(), &addr) < 0) {
		if (net_start_again() == 0)
			goto restart;
		else
			return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}
