// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <console.h>
#include <dm/device.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <lwip/icmp.h>
#include <lwip/inet_chksum.h>
#include <lwip/prot/icmp6.h>
#include <lwip/prot/ip6.h>
#include <lwip/raw.h>
#include <lwip/timeouts.h>
#include <net.h>
#include <time.h>

#define PING_DELAY_MS 1000
#define PING_COUNT 5
/* Ping identifier - must fit on a u16_t */
#define PING_ID 0xAFAF

struct ping_ctx {
	ip_addr_t target;
	struct raw_pcb *pcb;
	uint16_t seq_num;
	bool alive;
};

static u8_t ping_recv(void *arg, struct raw_pcb *pcb, struct pbuf *p,
		      const ip_addr_t *addr)
{
	struct ping_ctx *ctx = arg;
	struct icmp_echo_hdr iecho;
	u16 hlen = ip_current_header_tot_len();
	u16 checksum;
	u8 type = IP_IS_V6(addr) ? ICMP6_TYPE_EREP : ICMP_ER;

	if (!ip_addr_eq(addr, &ctx->target))
		return 0;

	if (pbuf_copy_partial(p, &iecho, sizeof(iecho), hlen) != sizeof(iecho) ||
	    iecho.type != type || iecho.code || iecho.id != PING_ID ||
	    iecho.seqno != lwip_htons(ctx->seq_num) || pbuf_remove_header(p, hlen))
		return 0;

	/* Raw callbacks run before the stack's ICMP checksum validation. */
	if (IP_IS_V6(addr))
		checksum = ip_chksum_pseudo(p, IP6_NEXTH_ICMP6, p->tot_len,
					    ip_current_src_addr(), ip_current_dest_addr());
	else
		checksum = inet_chksum_pbuf(p);
	if (checksum) {
		pbuf_add_header(p, hlen);
		return 0;
	}

	ctx->alive = true;
	printf("host %s is alive\n", ipaddr_ntoa(addr));
	pbuf_free(p);
	return 1;
}

static int ping_raw_init(struct ping_ctx *ctx, struct netif *netif)
{
	ctx->pcb = raw_new_ip_type(IP_GET_TYPE(&ctx->target),
				   IP_IS_V6(&ctx->target) ? IP6_NEXTH_ICMP6 : IP_PROTO_ICMP);
	if (!ctx->pcb)
		return -ENOMEM;

	raw_recv(ctx->pcb, ping_recv, ctx);
	raw_bind_netif(ctx->pcb, netif);
#if CONFIG_IS_ENABLED(IPV6)
	if (IP_IS_V6(&ctx->target)) {
		ctx->pcb->chksum_reqd = 1;
		ctx->pcb->chksum_offset = offsetof(struct icmp_echo_hdr, chksum);
	}
#endif

	return 0;
}

static void ping_raw_stop(struct ping_ctx *ctx)
{
	if (ctx->pcb)
		raw_remove(ctx->pcb);
}

static void ping_prepare_echo(struct ping_ctx *ctx, struct icmp_echo_hdr *iecho)
{
	ICMPH_TYPE_SET(iecho, IP_IS_V6(&ctx->target) ? ICMP6_TYPE_EREQ : ICMP_ECHO);
	ICMPH_CODE_SET(iecho, 0);
	iecho->chksum = 0;
	iecho->id = PING_ID;
	iecho->seqno = lwip_htons(ctx->seq_num);

	if (!IP_IS_V6(&ctx->target))
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
		ping_prepare_echo(ctx, p->payload);
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

static int ping_loop(struct net_lwip_ctx *net, const ip_addr_t *addr)
{
	struct ping_ctx ctx = {};
	int ret;

	printf("Using %s device\n", net->dev->name);

	ctx.target = *addr;
#if CONFIG_IS_ENABLED(IPV6)
	if (IP_IS_V6(&ctx.target)) {
		ret = net_lwip_ip6_prepare(net, &ctx.target);
		if (ret)
			return ret;
	}
#endif
	ret = ping_raw_init(&ctx, net->netif);
	if (ret < 0)
		return ret;

	ping_send(&ctx);

	do {
		ret = net_lwip_poll();
		if (ret < 0)
			break;
		if (ctx.alive)
			break;
		if (ctrlc()) {
			printf("\nAbort\n");
			ret = -EINTR;
			break;
		}
	} while (ctx.seq_num <= PING_COUNT);

	sys_untimeout(ping_send, &ctx);
	ping_raw_stop(&ctx);

	if (ctx.alive)
		return 0;

	printf("ping failed; host %s is not alive\n", ipaddr_ntoa(addr));
	return ret < 0 ? ret : -ETIMEDOUT;
}

static int ping_execute(const ip_addr_t *addr)
{
	struct net_lwip_ctx net = {};
	int ret;

	net_try_count = 1;

	do {
		ret = net_lwip_start(&net, IP_IS_V6(addr) ? NET_LWIP_ADDR_ENV_FLEXIBLE :
				    NET_LWIP_ADDR_ENV_STRICT);
		if (!ret)
			break;
	} while (!net_start_again());
	if (ret)
		return CMD_RET_FAILURE;

	do {
		ret = ping_loop(&net, addr);
		if (!ret || ret == -EINTR || ret == -EINVAL || ret == -EADDRINUSE ||
		    ret == -EBUSY)
			break;
	} while (!net_lwip_restart(&net));

	net_lwip_stop(&net);
	if (!ret)
		return CMD_RET_SUCCESS;

	return CMD_RET_FAILURE;
}

#if CONFIG_IS_ENABLED(CMD_PING)
int do_ping(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ip_addr_t addr;

	if (argc != 2 || net_lwip_dns_resolve(argv[1], &addr) || !IP_IS_V4(&addr))
		return CMD_RET_USAGE;

	return ping_execute(&addr);
}

U_BOOT_CMD(ping, 2, 1, do_ping, "send ICMP ECHO_REQUEST to network host",
	   "pingAddressOrHostName");
#endif

#if CONFIG_IS_ENABLED(CMD_PING6)
static int do_ping6(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ip_addr_t addr;

	if (argc != 2 || net_lwip_ip6_aton(argv[1], ip_2_ip6(&addr)))
		return CMD_RET_USAGE;
	IP_SET_TYPE_VAL(addr, IPADDR_TYPE_V6);

	return ping_execute(&addr);
}

U_BOOT_CMD(ping6, 2, 1, do_ping6, "send ICMPv6 ECHO_REQUEST to network host",
	   "IPv6address");
#endif
