// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2026 James Hilliard <james.hilliard1@gmail.com> */

#include <env.h>
#include <lwip/etharp.h>
#include <lwip/ip4_addr.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <net.h>
#include <timer.h>
#include <linux/errno.h>

#include "netconsole.h"

#define NC_ARP_TIMEOUT_MS	5000

static struct net_lwip_ctx nc_net;
static struct udp_pcb *nc_pcb;
static unsigned int nc_users;
static int nc_env_id = -1;
static ip_addr_t nc_dest;

static void nc_lwip_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
			 const ip_addr_t *addr, u16_t port)
{
	struct pbuf *q;

	if (IP_GET_TYPE(addr) != IP_GET_TYPE(&nc_dest))
		goto out;
	if (IP_IS_V6(addr)) {
		if (!ip_addr_cmp(addr, &nc_dest))
			goto out;
	} else {
		struct in_addr peer = { .s_addr = ip4_addr_get_u32(ip_2_ip4(&nc_dest)) };

		if (!ip_addr_cmp(addr, &nc_dest) && !nc_is_broadcast(peer))
			goto out;
	}
	for (q = p; q; q = q->next)
		nc_input_data(q->payload, q->len);
out:
	pbuf_free(p);
}

static int nc_lwip_refresh(bool prepare)
{
	struct udp_pcb *pcb;
	int env_id;
	err_t err;
	int ret;

	env_id = env_get_id();
	ret = nc_refresh_settings_from_env();
	if (ret < 0)
		return ret;

	if (env_id != nc_env_id) {
		ret = net_lwip_refresh(&nc_net);
		if (ret)
			return ret;
		ip_addr_set_ip4_u32_val(nc_dest, nc_settings.ip.s_addr);
#if CONFIG_IS_ENABLED(IPV6)
		if (nc_settings.ipv6) {
			ip_addr_copy_from_ip6(nc_dest, nc_settings.ip6);
			ip6_addr_assign_zone(ip_2_ip6(&nc_dest), IP6_UNICAST, nc_net.netif);
			ret = net_lwip_ip6_init(&nc_net);
			if (ret)
				return ret;
		}
#endif
		nc_env_id = env_id;
	}

	if (prepare) {
		ret = net_lwip_prepare_addr(&nc_net, &nc_dest);
		if (ret)
			return ret;
	}

	if (nc_pcb && nc_pcb->local_port == nc_settings.in_port)
		return 0;

	pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb)
		return -ENOMEM;

	err = udp_bind(pcb, IP_ANY_TYPE, nc_settings.in_port);
	if (err != ERR_OK) {
		udp_remove(pcb);
		return err;
	}

	ip_set_option(pcb, SOF_BROADCAST);
	udp_bind_netif(pcb, nc_net.netif);
	udp_recv(pcb, nc_lwip_recv, NULL);
	if (nc_pcb)
		udp_remove(nc_pcb);
	nc_pcb = pcb;

	return 0;
}

static bool nc_lwip_arp_target(const ip4_addr_t *dest, ip4_addr_t *target)
{
	struct netif *netif = nc_net.netif;

	if (ip4_addr_isbroadcast(dest, netif) || ip4_addr_ismulticast(dest))
		return false;

	ip4_addr_copy(*target, *dest);
	if (ip4_addr_net_eq(dest, netif_ip4_addr(netif),
			    netif_ip4_netmask(netif)) ||
	    ip4_addr_islinklocal(dest))
		return true;

	if (ip4_addr_isany_val(*netif_ip4_gw(netif)))
		return false;
	ip4_addr_copy(*target, *netif_ip4_gw(netif));

	return true;
}

static void nc_lwip_wait_for_arp(const ip4_addr_t *target)
{
	const ip4_addr_t *ip_ret;
	struct eth_addr *eth_ret;
	ulong start;

	if (etharp_find_addr(nc_net.netif, target, &eth_ret, &ip_ret) >= 0)
		return;

	start = get_timer(0);
	while (get_timer(start) < NC_ARP_TIMEOUT_MS) {
		if (net_lwip_poll() < 0)
			return;
		if (etharp_find_addr(nc_net.netif, target, &eth_ret,
				     &ip_ret) >= 0)
			return;
	}
}

int nc_transport_start(void)
{
	int ret;

	if (nc_users) {
		ret = nc_lwip_refresh(true);
		if (!ret)
			nc_users++;

		return ret;
	}

	ret = net_lwip_start(&nc_net, NET_LWIP_ADDR_ENV_FLEXIBLE);
	if (ret)
		return ret;

	ret = nc_lwip_refresh(true);
	if (ret) {
		net_lwip_stop(&nc_net);
		return ret;
	}

	nc_users = 1;

	return 0;
}

int nc_transport_stop(void)
{
	if (!nc_users)
		return 0;
	if (--nc_users)
		return 0;

	if (nc_pcb) {
		udp_remove(nc_pcb);
		nc_pcb = NULL;
	}

	net_lwip_stop(&nc_net);
	nc_env_id = -1;

	return 0;
}

void nc_transport_send(const char *buf, int len)
{
	ip4_addr_t arp_target;
	struct pbuf *p;
	err_t err;

	if (!nc_users || nc_lwip_refresh(false))
		return;

#if CONFIG_IS_ENABLED(IPV6)
	if (IP_IS_V6(&nc_dest)) {
		/* Output-only consoles also need lifetime and lease maintenance. */
		int ret = net_lwip_poll();

		if (ret < 0 && ret != -EBUSY)
			return;
		if (net_lwip_ip6_ready(&nc_net, &nc_dest))
			return;
	}
#endif

	p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
	if (!p)
		return;

	err = pbuf_take(p, buf, len);
	if (err != ERR_OK)
		goto out;

	err = udp_sendto_if(nc_pcb, p, &nc_dest, nc_settings.out_port,
			    nc_net.netif);
	if (err == ERR_OK) {
		if (IP_IS_V4(&nc_dest) &&
		    nc_lwip_arp_target(ip_2_ip4(&nc_dest), &arp_target))
			nc_lwip_wait_for_arp(&arp_target);
#if CONFIG_IS_ENABLED(IPV6)
		/* ND retains this RAM pbuf while the neighbor is unresolved. */
		if (IP_IS_V6(&nc_dest)) {
			ulong start = get_timer(0);

			while (p->ref > 1 && get_timer(start) < NC_ARP_TIMEOUT_MS) {
				if (net_lwip_poll() < 0)
					break;
			}
		}
#endif
	}

out:
	pbuf_free(p);
}

void nc_transport_poll(bool block)
{
	if (!nc_users || nc_lwip_refresh(false))
		return;

	do {
		if (net_lwip_poll() < 0)
			return;
	} while (block && !nc_has_input());
}
