// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <log.h>
#include <net.h>

#include "netconsole.h"

static int net_timeout;
static uchar nc_ether[ARP_HLEN];
static const char *output_packet;
static int output_packet_len;

/*
 * Start with a default last protocol.
 * We are only interested in NETCONS or not.
 */
enum proto_t net_loop_last_protocol = BOOTP;

static void nc_wait_arp_handler(uchar *pkt, unsigned int dest,
				struct in_addr sip, unsigned int src,
				unsigned int len)
{
	net_set_state(NETLOOP_SUCCESS);
}

static void nc_handler(uchar *pkt, unsigned int dest, struct in_addr sip,
		       unsigned int src, unsigned int len)
{
	if (nc_has_input())
		net_set_state(NETLOOP_SUCCESS);
}

static void nc_timeout_handler(void)
{
	net_set_state(NETLOOP_SUCCESS);
}

static int nc_legacy_refresh_settings(void)
{
	int ret;

	ret = nc_refresh_settings_from_env();
	if (ret <= 0)
		return ret;

	if (nc_is_broadcast(nc_settings.ip))
		memset(nc_ether, 0xff, sizeof(nc_ether));
	else
		memset(nc_ether, 0, sizeof(nc_ether));

	return 0;
}

/**
 * nc_start() - Configure the legacy network loop for netconsole
 *
 * Called from net_loop() before processing each packet.
 */
void nc_start(void)
{
	nc_legacy_refresh_settings();
	if (!output_packet_len || memcmp(nc_ether, net_null_ethaddr, ARP_HLEN)) {
		net_set_udp_handler(nc_handler);
		net_set_timeout_handler(net_timeout, nc_timeout_handler);
	} else {
		uchar *pkt;

		net_set_arp_handler(nc_wait_arp_handler);
		pkt = (uchar *)net_tx_packet + net_eth_hdr_size() +
			IP_UDP_HDR_SIZE;
		memcpy(pkt, output_packet, output_packet_len);
		net_send_udp_packet(nc_ether, nc_settings.ip,
				    nc_settings.out_port, nc_settings.in_port,
				    output_packet_len);
	}
}

void nc_transport_send(const char *buf, int len)
{
	struct udevice *eth;
	int inited = 0;
	uchar *pkt;

	debug_cond(DEBUG_DEV_PKT, "output: \"%*.*s\"\n", len, len, buf);

	eth = eth_get_dev();
	if (!eth)
		return;

	if (!memcmp(nc_ether, net_null_ethaddr, ARP_HLEN)) {
		if (eth_is_active(eth))
			return;
		output_packet = buf;
		output_packet_len = len;
		nc_set_input_recursion(true);
		net_loop(NETCONS);
		nc_set_input_recursion(false);
		output_packet_len = 0;
		return;
	}

	if (!eth_is_active(eth)) {
		if (eth_is_on_demand_init()) {
			if (eth_init() < 0)
				return;
			eth_set_last_protocol(NETCONS);
		} else {
			eth_init_state_only();
		}

		inited = 1;
	}

	pkt = (uchar *)net_tx_packet + net_eth_hdr_size() + IP_UDP_HDR_SIZE;
	memcpy(pkt, buf, len);
	net_send_udp_packet(nc_ether, nc_settings.ip, nc_settings.out_port,
			    nc_settings.in_port, len);

	if (inited) {
		if (eth_is_on_demand_init())
			eth_halt();
		else
			eth_halt_state_only();
	}
}

void nc_transport_poll(bool block)
{
	struct udevice *eth = eth_get_dev();

	if (eth_is_active(eth))
		return;

	net_timeout = block ? 0 : 1;
	net_loop(NETCONS);
}

int nc_transport_start(void)
{
	int ret;

	ret = nc_legacy_refresh_settings();
	if (ret)
		return ret;

	return net_init();
}

int nc_transport_stop(void)
{
	return 0;
}
