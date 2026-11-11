/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2026 James Hilliard <james.hilliard1@gmail.com> */

#ifndef __UBOOT_LWIP_HOOKS_H
#define __UBOOT_LWIP_HOOKS_H

#ifdef CONFIG_CMD_DHCP
#include "lwip-dhcp.h"
#endif

#if LWIP_IPV6
#include <lwip/ip6_addr.h>

struct netif;

void net_lwip_ip6_update_lifetimes(void);
#define LWIP_HOOK_ND6_UPDATE_LIFETIMES net_lwip_ip6_update_lifetimes

int net_lwip_ip6_static_prefix(struct netif *netif, unsigned int index,
			       const ip6_addr_t *dest);
#define LWIP_HOOK_IP6_STATIC_PREFIX_MATCH net_lwip_ip6_static_prefix

const ip6_addr_t *net_lwip_ip6_gateway(struct netif *netif,
				       const ip6_addr_t *dest);
#define LWIP_HOOK_ND6_GET_GW net_lwip_ip6_gateway
#ifdef CONFIG_CMD_DHCP6
/* DHCPv6 owns global-address lifetimes while SLAAC is disabled. */
#define LWIP_HOOK_ND6_AUTOCONFIG(netif) ((netif)->ip6_autoconfig_enabled)
#endif
#ifdef CONFIG_PROT_ICMP_LWIP
struct pbuf;
void net_lwip_icmp6_dest_unreach(unsigned int code, struct pbuf *p);
#define LWIP_HOOK_ICMP6_DEST_UNREACH net_lwip_icmp6_dest_unreach
#endif
#if LWIP_ND6_RDNSS_MAX_DNS_SERVERS
int net_lwip_rdnss_accept(struct netif *netif);
#define LWIP_HOOK_ND6_RDNSS_ACCEPT net_lwip_rdnss_accept
#endif
#endif

#endif /* __UBOOT_LWIP_HOOKS_H */
