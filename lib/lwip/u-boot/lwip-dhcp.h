/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2026 James Hilliard <james.hilliard1@gmail.com> */

#ifndef __UBOOT_LWIP_DHCP_H
#define __UBOOT_LWIP_DHCP_H

#include <lwip/err.h>

struct dhcp;
struct dhcp_msg;
struct netif;
struct pbuf;

err_t net_lwip_dhcp_ack(struct netif *netif, struct dhcp *dhcp, struct pbuf *p);
void net_lwip_dhcp_append(struct netif *netif, struct dhcp *dhcp, u8_t state,
			  struct dhcp_msg *msg, u8_t type, u16_t *len);

#define LWIP_HOOK_DHCP_HANDLE_ACK net_lwip_dhcp_ack
#define LWIP_HOOK_DHCP_APPEND_OPTIONS net_lwip_dhcp_append

#endif /* __UBOOT_LWIP_DHCP_H */
