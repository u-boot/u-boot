// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2026 James Hilliard <james.hilliard1@gmail.com> */

#include <dm.h>
#include <env.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <net.h>
#include <stdio_dev.h>
#include <asm/eth.h>
#include <dm/test.h>
#include <linux/errno.h>
#include <test/ut.h>

#define NC_TEST_MAX_OUTPUTS	8
#define NC_TEST_MAX_PAYLOAD	32
#define NC_TEST_SERVICE_PORT	12000

struct nc_test_eth_hdr {
	u8 dst[ARP_HLEN];
	u8 src[ARP_HLEN];
	__be16 type;
} __packed;

struct nc_test_output {
	struct in_addr src_ip;
	struct in_addr dest_ip;
	u16 src_port;
	u16 dest_port;
	char payload[NC_TEST_MAX_PAYLOAD];
	unsigned int len;
};

struct nc_test_context {
	struct nc_test_output output[NC_TEST_MAX_OUTPUTS];
	unsigned int output_count;
	int error;
};

struct nc_test_service_context {
	struct stdio_dev *nc;
	const char *message;
	unsigned int packets;
	int nested_poll_ret;
};

static int nc_test_tx_handler(struct udevice *dev, void *packet,
			      unsigned int len)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct nc_test_context *context = priv->priv;
	struct nc_test_eth_hdr *eth = packet;
	struct nc_test_output *output;
	struct ip_udp_hdr *ip;
	unsigned int payload_len;

	if (!sandbox_eth_arp_req_to_reply(dev, packet, len))
		return 0;
	if (!sandbox_eth_ping_req_to_reply(dev, packet, len))
		return 0;

	if (len < sizeof(*eth) || ntohs(eth->type) != PROT_IP)
		return 0;
	if (len < sizeof(*eth) + IP_UDP_HDR_SIZE) {
		context->error = -EINVAL;
		return 0;
	}

	ip = (void *)(eth + 1);
	if (ip->ip_p != IPPROTO_UDP)
		return 0;
	if (ntohs(ip->udp_len) < UDP_HDR_SIZE ||
	    ntohs(ip->udp_len) > len - sizeof(*eth) - IP_HDR_SIZE) {
		context->error = -EINVAL;
		return 0;
	}

	payload_len = ntohs(ip->udp_len) - UDP_HDR_SIZE;
	if (context->output_count == NC_TEST_MAX_OUTPUTS ||
	    payload_len > NC_TEST_MAX_PAYLOAD) {
		context->error = -ENOSPC;
		return 0;
	}

	output = &context->output[context->output_count++];
	output->src_ip = ip->ip_src;
	output->dest_ip = ip->ip_dst;
	output->src_port = ntohs(ip->udp_src);
	output->dest_port = ntohs(ip->udp_dst);
	output->len = payload_len;
	memcpy(output->payload, ip + 1, payload_len);

	return 0;
}

static int nc_test_inject(struct udevice *dev, const char *payload,
			  struct in_addr src_ip, struct in_addr dest_ip,
			  u16 src_port, u16 dest_port)
{
	struct eth_sandbox_priv *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct nc_test_eth_hdr *eth;
	struct ip_udp_hdr *ip;
	unsigned int payload_len = strlen(payload);
	unsigned int packet_len;

	packet_len = sizeof(*eth) + IP_UDP_HDR_SIZE + payload_len;
	if (priv->recv_packets >= PKTBUFSRX || packet_len > PKTSIZE_ALIGN)
		return -ENOSPC;

	eth = (void *)priv->recv_packet_buffer[priv->recv_packets];
	if (dest_ip.s_addr == (u32)~0)
		memset(eth->dst, 0xff, ARP_HLEN);
	else
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
	ip->ip_dst = dest_ip;
	ip->udp_src = htons(src_port);
	ip->udp_dst = htons(dest_port);
	ip->udp_len = htons(UDP_HDR_SIZE + payload_len);
	ip->ip_sum = compute_ip_checksum(ip, IP_HDR_SIZE);
	memcpy(ip + 1, payload, payload_len);

	priv->recv_packet_length[priv->recv_packets] = packet_len;
	priv->recv_packets++;

	return 0;
}

static void nc_test_service_recv(void *arg, struct udp_pcb *pcb,
				 struct pbuf *p, const ip_addr_t *addr,
				 u16_t port)
{
	struct nc_test_service_context *context = arg;

	context->packets++;
	context->nc->puts(context->nc, context->message);
	context->nested_poll_ret = net_lwip_poll();
	pbuf_free(p);
}

static int nc_test_check_output(struct unit_test_state *uts,
				struct nc_test_context *context,
				unsigned int index, const char *payload,
				const char *dest_ip, u16 src_port,
				u16 dest_port)
{
	struct nc_test_output *output;

	ut_asserteq(0, context->error);
	ut_assert(index < context->output_count);
	output = &context->output[index];
	ut_asserteq(strlen(payload), output->len);
	ut_asserteq_mem(payload, output->payload, output->len);
	ut_asserteq(string_to_ip("1.1.2.1").s_addr,
		    output->src_ip.s_addr);
	ut_asserteq(string_to_ip(dest_ip).s_addr, output->dest_ip.s_addr);
	ut_asserteq(src_port, output->src_port);
	ut_asserteq(dest_port, output->dest_port);

	return 0;
}

static int nc_test_read_input(struct unit_test_state *uts,
			      struct stdio_dev *nc, const char *expected)
{
	unsigned int i;

	ut_assert(nc->tstc(nc));
	for (i = 0; expected[i]; i++)
		ut_asserteq(expected[i], nc->getc(nc));
	ut_assert(!nc->tstc(nc));

	return 0;
}

static int _dm_test_lwip_netconsole(struct unit_test_state *uts,
				    struct stdio_dev *nc,
				    struct udevice *dev,
				    struct nc_test_context *context,
				    struct net_lwip_ctx *service_net,
				    struct udp_pcb **service_pcb)
{
	static const char output[] = "netconsole output";
	static const char input[] = "netconsole input";
	static const char broadcast_output[] = "broadcast output";
	static const char broadcast_input[] = "broadcast input";
	static const char after_ping[] = "after ping";
	static const char from_service[] = "from service";
	static const char after_service[] = "after service";
	static const char changed_input[] = "changed input";
	static const char default_output[] = "default output";
	static const char default_input[] = "default input";
	struct nc_test_service_context service = {
		.nc = nc,
		.message = from_service,
	};
	struct in_addr host = string_to_ip("1.1.2.2");
	unsigned int outputs;

	ut_assertok(env_set("ethact", "eth@10002000"));
	ut_assertok(env_set("ipaddr", "1.1.2.1"));
	ut_assertok(env_set("netmask", "255.255.255.0"));
	ut_assertok(env_set("ncinport", "40000"));
	ut_assertok(env_set("ncoutport", "40001"));

	ut_assertok(nc->start(nc));
	ut_assertok(nc->start(nc));
	nc->stop(nc);
	nc->puts(nc, broadcast_output);
	ut_asserteq(1, context->output_count);
	ut_assertok(nc_test_check_output(uts, context, 0, broadcast_output,
					 "255.255.255.255", 40000, 40001));
	ut_assertok(nc_test_inject(dev, broadcast_input, host,
				   string_to_ip("255.255.255.255"), 40001,
				   40000));
	ut_assertok(nc_test_read_input(uts, nc, broadcast_input));
	outputs = 1;

	ut_assertok(env_set("ncip", "1.1.2.2"));
	nc->puts(nc, output);
	ut_asserteq(++outputs, context->output_count);
	ut_assertok(nc_test_check_output(uts, context, outputs - 1, output,
					 "1.1.2.2", 40000, 40001));

	ut_assertok(nc_test_inject(dev, input, host,
				   string_to_ip("1.1.2.1"), 40001, 40000));
	ut_assertok(nc_test_read_input(uts, nc, input));

	if (IS_ENABLED(CONFIG_CMD_PING)) {
		static char *const ping_argv[] = { "ping", "1.1.2.2" };

		/* A foreground command shares the netif with netconsole. */
		ut_assertok(do_ping(NULL, 0, ARRAY_SIZE(ping_argv), ping_argv));
		nc->puts(nc, after_ping);
		ut_asserteq(++outputs, context->output_count);
		ut_assertok(nc_test_check_output(uts, context, outputs - 1,
						 after_ping,
						 "1.1.2.2", 40000, 40001));
	}

	/* Model an independent server protocol attached to the same runtime. */
	ut_assertok(net_lwip_start(service_net,
				   NET_LWIP_ADDR_ENV_STRICT));
	*service_pcb = udp_new();
	ut_assertnonnull(*service_pcb);
	ut_asserteq(ERR_OK, udp_bind(*service_pcb, IP_ADDR_ANY,
				     NC_TEST_SERVICE_PORT));
	udp_recv(*service_pcb, nc_test_service_recv, &service);

	/*
	 * Change to an uncached peer before entering the service callback. The
	 * callback's netconsole output starts ARP resolution. The outer runtime
	 * poll must receive the reply and flush the queued datagram without a
	 * recursive poll.
	 */
	ut_assertok(env_set("ncip", "1.1.2.3"));
	ut_assertok(env_set("ncinport", "50000"));
	ut_assertok(env_set("ncoutport", "50001"));
	ut_assertok(nc_test_inject(dev, "request", host,
				   string_to_ip("1.1.2.1"), 13000,
				   NC_TEST_SERVICE_PORT));
	ut_assertok(net_lwip_poll());
	ut_asserteq(1, service.packets);
	ut_asserteq(-EBUSY, service.nested_poll_ret);
	ut_asserteq(++outputs, context->output_count);
	ut_assertok(nc_test_check_output(uts, context, outputs - 1,
					 from_service,
					 "1.1.2.3", 50000, 50001));

	nc->puts(nc, after_service);
	ut_asserteq(++outputs, context->output_count);
	ut_assertok(nc_test_check_output(uts, context, outputs - 1,
					 after_service,
					 "1.1.2.3", 50000, 50001));

	ut_assertok(nc_test_inject(dev, changed_input,
				   string_to_ip("1.1.2.3"),
				   string_to_ip("1.1.2.1"), 50001, 50000));
	ut_assertok(nc_test_read_input(uts, nc, changed_input));

	ut_assertok(env_set("ncip", NULL));
	ut_assertok(env_set("ncinport", NULL));
	ut_assertok(env_set("ncoutport", NULL));
	nc->puts(nc, default_output);
	ut_asserteq(++outputs, context->output_count);
	ut_assertok(nc_test_check_output(uts, context, outputs - 1,
					 default_output, "255.255.255.255",
					 6666, 6666));
	ut_assertok(nc_test_inject(dev, default_input, host,
				   string_to_ip("255.255.255.255"), 6666,
				   6666));
	ut_assertok(nc_test_read_input(uts, nc, default_input));

	return 0;
}

static int dm_test_lwip_netconsole(struct unit_test_state *uts)
{
	struct nc_test_context context = {};
	struct net_lwip_ctx service_net = {};
	struct udp_pcb *service_pcb = NULL;
	struct stdio_dev *nc;
	struct udevice *dev;
	int ret;

	nc = stdio_get_by_name("nc");
	ut_assertnonnull(nc);
	ut_assertnonnull(nc->stop);
	ut_assertok(uclass_get_device_by_name(UCLASS_ETH, "eth@10002000",
					      &dev));

	sandbox_eth_set_tx_handler(0, nc_test_tx_handler);
	sandbox_eth_set_priv(0, &context);
	ret = _dm_test_lwip_netconsole(uts, nc, dev, &context, &service_net,
				       &service_pcb);
	if (service_pcb)
		udp_remove(service_pcb);
	net_lwip_stop(&service_net);
	nc->stop(nc);
	sandbox_eth_set_tx_handler(0, NULL);
	sandbox_eth_set_priv(0, NULL);

	env_set("ethact", NULL);
	env_set("ipaddr", "192.0.2.1");
	env_set("netmask", NULL);
	env_set("ncip", NULL);
	env_set("ncinport", NULL);
	env_set("ncoutport", NULL);

	return ret;
}

DM_TEST(dm_test_lwip_netconsole, UTF_SCAN_FDT);
