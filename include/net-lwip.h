/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __NET_LWIP_H__
#define __NET_LWIP_H__

#include <lwip/ip4.h>
#include <lwip/netif.h>

struct netif *net_lwip_new_netif(struct udevice *udev);
struct netif *net_lwip_new_netif_noip(struct udevice *udev);
void net_lwip_remove_netif(struct netif *netif);
struct netif *net_lwip_get_netif(void);

#endif /* __NET_LWIP_H__ */
