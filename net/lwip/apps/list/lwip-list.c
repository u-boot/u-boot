// SPDX-License-Identifier: GPL-2.0

/*
 * (C) Copyright 2023 Linaro Ltd. <maxim.uvarov@linaro.org>
 */

#include <common.h>
#include <command.h>
#include <console.h>

#include <lwip/ip_addr.h>
#include <lwip/netif.h>
#include <net/lwip.h>

void ulwip_up_down(char *name, int up)
{
	struct netif *netif;

	NETIF_FOREACH(netif) {
		char iname[6];

		snprintf(iname, 6, "eth%s", netif->name);
		if (!strncmp(iname, name, 6)) {
			if (up)
				netif_set_up(netif);
			else {
				netif_set_down(netif);
				//netif_remove(netif);
			}
			log_info("swiched %s to %s\n", iname, up ? "UP" : "DOWN");
			break;
		}
	}
}

void ulwip_list(void)
{
	struct netif *netif;

	NETIF_FOREACH(netif) {
		char ipstr[IP4ADDR_STRLEN_MAX];
		char maskstr[IP4ADDR_STRLEN_MAX];
		char gwstr[IP4ADDR_STRLEN_MAX];

		log_info("%d:  eth%s %s IP: %s/%s, GW: %s, %pM\n",
			netif_get_index(netif),
			netif->name,
			netif_is_up(netif) == 1 ? "UP" : "DOWN",
			ip4addr_ntoa_r(&netif->ip_addr, ipstr, IP4ADDR_STRLEN_MAX),
			ip4addr_ntoa_r(&netif->netmask, maskstr, IP4ADDR_STRLEN_MAX),
			ip4addr_ntoa_r(&netif->gw, gwstr, IP4ADDR_STRLEN_MAX),
			netif->hwaddr);
	}

	log_info("done.\n");
}
