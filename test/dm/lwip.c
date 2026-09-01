// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2026 James Hilliard <james.hilliard1@gmail.com> */

#include <dm.h>
#include <env.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <net.h>
#include <asm/eth.h>
#include <dm/test.h>
#include <linux/kconfig.h>
#include <test/ut.h>

#if CONFIG_IS_ENABLED(PROT_DNS_LWIP)
#include <lwip/dns.h>
#endif
#if CONFIG_IS_ENABLED(PROT_DHCP_LWIP)
#include <lwip/dhcp.h>
#include <lwip/prot/dhcp.h>
#endif
#if CONFIG_IS_ENABLED(WGET)
#include <lwip/apps/http_client.h>
#include <lwip/priv/tcp_priv.h>
#endif

struct lwip_test_eth_hdr {
	u8 dst[ARP_HLEN];
	u8 src[ARP_HLEN];
	__be16 type;
} __packed;

struct lwip_test_recv_ctx {
	unsigned int packets;
	int nested_poll_ret;
};

#if CONFIG_IS_ENABLED(PROT_DNS_LWIP)
struct lwip_test_dns_ctx {
	unsigned int callbacks;
	bool address_missing;
};

static void lwip_test_dns_found(const char *name, const ip_addr_t *ipaddr,
				void *arg)
{
	struct lwip_test_dns_ctx *ctx = arg;

	ctx->callbacks++;
	ctx->address_missing = !ipaddr;
}
#endif

#if CONFIG_IS_ENABLED(WGET)
struct lwip_test_http_ctx {
	unsigned int callbacks;
	httpc_result_t result;
};

static void lwip_test_http_result(void *arg, httpc_result_t result,
				  u32_t rx_content_len, u32_t server_response,
				  err_t err)
{
	struct lwip_test_http_ctx *ctx = arg;

	ctx->callbacks++;
	ctx->result = result;
}

static err_t lwip_test_http_recv(void *arg, struct altcp_pcb *conn,
				 struct pbuf *p, err_t err)
{
	if (p)
		pbuf_free(p);

	return ERR_OK;
}
#endif

static void lwip_test_udp_recv(void *arg, struct udp_pcb *pcb,
			       struct pbuf *p, const ip_addr_t *addr,
			       u16_t port)
{
	struct lwip_test_recv_ctx *recv_ctx = arg;

	recv_ctx->packets++;
	recv_ctx->nested_poll_ret = net_lwip_poll();
	pbuf_free(p);
}

#if CONFIG_IS_ENABLED(PROT_DHCP_LWIP)
static int lwip_test_dhcp_stop(struct unit_test_state *uts,
			       struct netif *netif)
{
	struct dhcp *dhcp;
	ip4_addr_t address;
	int i;

	ip4addr_aton("1.1.2.5", &address);
	for (i = 0; i < 2; i++) {
		ut_asserteq(ERR_OK, dhcp_start(netif));
		dhcp = netif_dhcp_data(netif);
		ut_assertnonnull(dhcp);
		dhcp->state = DHCP_STATE_BOUND;
		netif_set_ipaddr(netif, &address);
		ut_assert(dhcp_supplied_address(netif));
		dhcp_stop_without_release(netif);
		ut_assert(ip4_addr_cmp(netif_ip4_addr(netif), &address));
		ut_asserteq(0, dhcp->pcb_allocated);
		dhcp_cleanup(netif);
		ut_assertnull(netif_dhcp_data(netif));
		ut_assertok(net_lwip_poll());
	}

	return 0;
}
#endif

static int lwip_test_inject_udp(struct udevice *dev, const char *payload,
				struct in_addr src_ip, u16 src_port,
				u16 dest_port)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct lwip_test_eth_hdr *eth;
	unsigned int payload_len = strlen(payload);
	unsigned int packet_len;
	struct ip_udp_hdr *ip;

	packet_len = sizeof(*eth) + IP_UDP_HDR_SIZE + payload_len;
	if (priv->recv_packets >= PKTBUFSRX || packet_len > PKTSIZE_ALIGN)
		return -ENOSPC;

	eth = (void *)priv->recv_packet_buffer[priv->recv_packets];
	memcpy(eth->dst, pdata->enetaddr, ARP_HLEN);
	memcpy(eth->src, priv->fake_host_hwaddr, ARP_HLEN);
	eth->type = htons(PROT_IP);

	ip = (void *)(eth + 1);
	memset(ip, 0, IP_UDP_HDR_SIZE);
	ip->ip_hl_v = 0x45;
	ip->ip_len = htons(IP_UDP_HDR_SIZE + payload_len);
	ip->ip_off = htons(IP_FLAGS_DFRAG);
	ip->ip_ttl = 64;
	ip->ip_p = IPPROTO_UDP;
	ip->ip_src = src_ip;
	ip->ip_dst = string_to_ip(env_get("ipaddr"));
	ip->udp_src = htons(src_port);
	ip->udp_dst = htons(dest_port);
	ip->udp_len = htons(UDP_HDR_SIZE + payload_len);
	ip->ip_sum = compute_ip_checksum(ip, IP_HDR_SIZE);
	memcpy(ip + 1, payload, payload_len);

	priv->recv_packet_length[priv->recv_packets] = packet_len;
	priv->recv_packets++;

	return 0;
}

#if CONFIG_IS_ENABLED(PROT_DNS_LWIP)
static int lwip_test_dns_cancel(struct unit_test_state *uts)
{
	struct lwip_test_dns_ctx cancelled = {};
	struct lwip_test_dns_ctx active = {};
	ip_addr_t dns_server;
	ip_addr_t result;
	int ret;
	int i;

	ipaddr_aton("1.1.2.2", &dns_server);
	dns_setserver(0, &dns_server);

	ret = dns_gethostbyname("cancel.test", &result,
				lwip_test_dns_found, &cancelled);
	ut_asserteq(ERR_INPROGRESS, ret);

	/* Start the query so a second request can share it. */
	dns_tmr();
	ut_assertok(net_lwip_poll());
	dns_cancel(lwip_test_dns_found, &cancelled);

	ret = dns_gethostbyname("cancel.test", &result,
				lwip_test_dns_found, &active);
	ut_asserteq(ERR_INPROGRESS, ret);

	for (i = 0; i < 16; i++)
		dns_tmr();

	ut_asserteq(0, cancelled.callbacks);
	ut_asserteq(1, active.callbacks);
	ut_assert(active.address_missing);

	return 0;
}
#endif

#if CONFIG_IS_ENABLED(WGET)
static int lwip_test_http_abort(struct unit_test_state *uts)
{
	struct lwip_test_http_ctx ctx = {};
	httpc_connection_t settings = {
		.result_fn = lwip_test_http_result,
	};
	httpc_state_t *state = NULL;
	ip_addr_t server;
	err_t err;
#if CONFIG_IS_ENABLED(PROT_DNS_LWIP)
	int i;

	err = httpc_get_file_dns("abort-http.test", HTTP_DEFAULT_PORT, "/",
				 &settings, lwip_test_http_recv, &ctx, &state);
	ut_asserteq(ERR_OK, err);
	ut_assertnonnull(state);
	ut_asserteq(ERR_OK, httpc_abort(state));
	ut_asserteq(1, ctx.callbacks);
	ut_asserteq(HTTPC_RESULT_LOCAL_ABORT, ctx.result);

	/* A later DNS timeout must not call into the freed HTTP state. */
	for (i = 0; i < 16; i++)
		dns_tmr();
	ut_asserteq(1, ctx.callbacks);

	ctx = (struct lwip_test_http_ctx){};
	state = NULL;
#endif
	ipaddr_aton("1.1.2.2", &server);
	err = httpc_get_file(&server, HTTP_DEFAULT_PORT, "/", &settings,
			     lwip_test_http_recv, &ctx, &state);
	ut_asserteq(ERR_OK, err);
	ut_assertnonnull(state);
	ut_assertnonnull(tcp_active_pcbs);
	ut_asserteq(SYN_SENT, tcp_active_pcbs->state);
	ut_assertnull(tcp_active_pcbs->next);

	/* An established connection must be removed, not left closing. */
	tcp_active_pcbs->state = ESTABLISHED;
	ut_asserteq(ERR_OK, httpc_abort(state));
	ut_asserteq(1, ctx.callbacks);
	ut_asserteq(HTTPC_RESULT_LOCAL_ABORT, ctx.result);
	ut_assertnull(tcp_active_pcbs);

	return 0;
}
#endif

static int _dm_test_lwip_runtime(struct unit_test_state *uts,
				 struct net_lwip_ctx *client_a,
				 struct net_lwip_ctx *client_b,
				 struct net_lwip_ctx *no_addr_client,
				 struct udp_pcb **pcb_a,
				 struct udp_pcb **pcb_b)
{
	struct lwip_test_recv_ctx recv_a = {};
	struct lwip_test_recv_ctx recv_b = {};
	struct in_addr host = string_to_ip("1.1.2.2");
	ip4_addr_t expected;

	ut_assertok(env_set("ethact", "eth@10002000"));
	ut_assertok(env_set("ipaddr", "1.1.2.1"));
	ut_assertok(env_set("netmask", "255.255.255.0"));
	ut_assertok(env_set("gatewayip", "1.1.2.254"));

	ut_assertok(net_lwip_start(client_a, NET_LWIP_ADDR_ENV_STRICT));
	ut_assertnonnull(client_a->dev);
	ut_assertnonnull(client_a->netif);
	ut_assert(eth_is_active(client_a->dev));
	ip4addr_aton("1.1.2.1", &expected);
	ut_assert(ip4_addr_cmp(netif_ip4_addr(client_a->netif), &expected));

	ut_assertok(net_lwip_start(client_b, NET_LWIP_ADDR_ENV_FLEXIBLE));
	ut_asserteq_ptr(client_a->dev, client_b->dev);
	ut_asserteq_ptr(client_a->netif, client_b->netif);

	/* Strict clients prevent every change to the shared IPv4 address. */
	ut_assertok(env_set("ipaddr", "1.1.2.3"));
	ut_asserteq(-EBUSY, net_lwip_refresh(client_b));
	ut_asserteq(-EBUSY,
		    net_lwip_start(no_addr_client, NET_LWIP_ADDR_NONE));
	ut_assertnull(no_addr_client->dev);
	ut_assertnull(no_addr_client->netif);
	ip4addr_aton("1.1.2.1", &expected);
	ut_assert(ip4_addr_cmp(netif_ip4_addr(client_a->netif), &expected));

	net_lwip_stop(client_a);
	ut_assertok(net_lwip_refresh(client_b));
	ip4addr_aton("1.1.2.3", &expected);
	ut_assert(ip4_addr_cmp(netif_ip4_addr(client_b->netif), &expected));

	ut_assertok(net_lwip_start(no_addr_client, NET_LWIP_ADDR_NONE));
	ut_assert(ip4_addr_isany_val(*netif_ip4_addr(client_b->netif)));
	ut_asserteq(-EBUSY,
		    net_lwip_start(client_a, NET_LWIP_ADDR_NONE));
	ut_assertnull(client_a->dev);
	ut_assertnull(client_a->netif);
	ut_asserteq(-EBUSY,
		    net_lwip_start(client_a, NET_LWIP_ADDR_ENV_STRICT));
	ut_assertnull(client_a->dev);
	ut_assertnull(client_a->netif);
	ut_assertok(env_set("ipaddr", "1.1.2.4"));
	ut_assertok(net_lwip_refresh(client_b));
	ut_assert(ip4_addr_isany_val(*netif_ip4_addr(client_b->netif)));
#if CONFIG_IS_ENABLED(PROT_DHCP_LWIP)
	ut_assertok(lwip_test_dhcp_stop(uts, no_addr_client->netif));
#endif
	net_lwip_stop(no_addr_client);
	ip4addr_aton("1.1.2.4", &expected);
	ut_assert(ip4_addr_cmp(netif_ip4_addr(client_b->netif), &expected));

	ut_assertok(net_lwip_start(client_a, NET_LWIP_ADDR_ENV_STRICT));
	ut_asserteq_ptr(client_a->dev, client_b->dev);
	ut_asserteq_ptr(client_a->netif, client_b->netif);
#if CONFIG_IS_ENABLED(PROT_DNS_LWIP)
	ut_assertok(lwip_test_dns_cancel(uts));
#endif
#if CONFIG_IS_ENABLED(WGET)
	ut_assertok(lwip_test_http_abort(uts));
#endif

	*pcb_a = udp_new();
	ut_assertnonnull(*pcb_a);
	ut_asserteq(ERR_OK, udp_bind(*pcb_a, IP_ADDR_ANY, 10000));
	udp_recv(*pcb_a, lwip_test_udp_recv, &recv_a);
	*pcb_b = udp_new();
	ut_assertnonnull(*pcb_b);
	ut_asserteq(ERR_OK, udp_bind(*pcb_b, IP_ADDR_ANY, 10001));
	udp_recv(*pcb_b, lwip_test_udp_recv, &recv_b);

	ut_assertok(lwip_test_inject_udp(client_a->dev, "one", host,
					 20000, 10000));
	ut_assertok(net_lwip_poll());
	ut_assertok(lwip_test_inject_udp(client_a->dev, "two", host,
					 20001, 10001));
	ut_assertok(net_lwip_poll());
	ut_asserteq(1, recv_a.packets);
	ut_asserteq(1, recv_b.packets);
	ut_asserteq(-EBUSY, recv_a.nested_poll_ret);
	ut_asserteq(-EBUSY, recv_b.nested_poll_ret);

	net_lwip_stop(client_a);
	ut_assertnonnull(client_b->netif);
	ut_assert(eth_is_active(client_b->dev));

	return 0;
}

static int dm_test_lwip_runtime(struct unit_test_state *uts)
{
	struct net_lwip_ctx no_addr_client = {};
	struct net_lwip_ctx client_a = {};
	struct net_lwip_ctx client_b = {};
	struct udp_pcb *pcb_a = NULL;
	struct udp_pcb *pcb_b = NULL;
	struct udevice *dev;
	int ret;

	ret = _dm_test_lwip_runtime(uts, &client_a, &client_b,
				    &no_addr_client, &pcb_a, &pcb_b);

	if (pcb_b)
		udp_remove(pcb_b);
	if (pcb_a)
		udp_remove(pcb_a);
	net_lwip_stop(&no_addr_client);
	net_lwip_stop(&client_a);
	dev = client_b.dev;
	net_lwip_stop(&client_b);

	if (!ret) {
		ut_assert(!eth_is_active(dev));
		ut_asserteq(-ENODEV, net_lwip_poll());
	}

	return ret;
}

DM_TEST(dm_test_lwip_runtime, UTF_SCAN_FDT);
