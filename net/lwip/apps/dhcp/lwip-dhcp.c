// SPDX-License-Identifier: GPL-2.0

/*
 * (C) Copyright 2023 Linaro Ltd. <maxim.uvarov@linaro.org>
 */

#include <common.h>
#include <command.h>
#include <console.h>

#include <lwip/dhcp.h>
#include <lwip/prot/dhcp.h>
#include "lwip/timeouts.h"

#include <net/eth.h>
#include <net/ulwip.h>
#include <dm/device.h>

#define DHCP_TMO_TIME 1000 /* poll for DHCP state change */
#define DHCP_TMO_NUM  20  /* number of tries */

typedef struct dhcp_priv {
	int num_tries;
	int net_idx;
	struct netif *netif;
} dhcp_priv;

static void dhcp_tmo(void *arg)
{
	struct dhcp_priv *dpriv = (struct dhcp_priv *)arg;
	struct netif *netif = dpriv->netif;
	struct dhcp *dhcp;

	log_info("%s %d/%d\n", __func__, dpriv->num_tries, DHCP_TMO_NUM);
	dhcp = netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);
	if (!dhcp)
		return;

	if (dhcp->state == DHCP_STATE_BOUND) {
		int err = 0;

		err -= env_set("bootfile", dhcp->boot_file_name);
		if (!dpriv->net_idx)  {
			err -= env_set("ipaddr", ip4addr_ntoa(&dhcp->offered_ip_addr));
			err -= env_set("netmask", ip4addr_ntoa(&dhcp->offered_sn_mask));
			err -= env_set("serverip", ip4addr_ntoa(&dhcp->server_ip_addr));
		} else {
			char var[10];

			snprintf(var, 10, "ipaddr%d", dpriv->net_idx);
			err -= env_set(var, ip4addr_ntoa(&dhcp->offered_ip_addr));
			snprintf(var, 10, "netmask%d", dpriv->net_idx);
			err -= env_set(var, ip4addr_ntoa(&dhcp->offered_sn_mask));
			snprintf(var, 10, "serverip%d", dpriv->net_idx);
			err -= env_set(var, ip4addr_ntoa(&dhcp->server_ip_addr));
		}
		if (err)
			log_err("error update envs\n");
		log_info("DHCP client bound to address %s\n", ip4addr_ntoa(&dhcp->offered_ip_addr));
		free(dpriv);
		ulwip_exit(err);
		return;
	}

	dpriv->num_tries--;
	if (dpriv->num_tries < 0) {
		log_err("DHCP client timeout\n");
		free(dpriv);
		ulwip_exit(-1);
		return;
	}

	sys_timeout(DHCP_TMO_TIME, dhcp_tmo, dpriv);
}

static struct netif *uidx_to_lwip_netif(int idx)
{
	struct netif *netif;
	char name[2];

	snprintf(name, 2, "%d", idx);

	NETIF_FOREACH(netif) {
		if (netif->name[0] == name[0] && netif->name[1] == name[1])
			return netif;
	}

	return NULL;
}

int ulwip_dhcp(void)
{
	struct netif *netif;
	struct dhcp_priv *dpriv;

	struct udevice *udev;

	dpriv = malloc(sizeof(struct dhcp_priv));
	if (!dpriv)
		return -EPERM;

	udev = eth_get_dev();
	if (!udev)
		return -ENOENT;

	log_info("dhcp using: %d:%s\n", udev->seq_, udev->name);

	netif = uidx_to_lwip_netif(dev_seq(udev));
	if (!netif)
		return -ENOENT;

	dpriv->num_tries = DHCP_TMO_NUM;
	dpriv->netif = netif;
	dpriv->net_idx = dev_seq(udev);
	sys_timeout(DHCP_TMO_TIME, dhcp_tmo, dpriv);

	return dhcp_start(netif) ? -EPERM : 0;
}
