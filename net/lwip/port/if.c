// SPDX-License-Identifier: GPL-2.0

/*
 * (C) Copyright 2023 Linaro Ltd. <maxim.uvarov@linaro.org>
 */

#include <common.h>
#include <command.h>
#include <net/eth.h>
#include <dm/device.h>
#include <dm/uclass-id.h>
#include <dm/uclass.h>
#include "lwip/debug.h"
#include "lwip/arch.h"
#include "netif/etharp.h"
#include "lwip/stats.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/netif.h"
#include "lwip/ethip6.h"
#include "lwip/timeouts.h"

#include "lwip/ip.h"

/*
 * MAC_ADDR_STRLEN: length of mac address string
 */
#define MAC_ADDR_STRLEN 18

int ulwip_active(void)
{
	struct udevice *udev;
	struct ulwip *ulwip;

	udev = eth_get_dev();
	if (!udev)
		return 0;

	ulwip = eth_lwip_priv(udev);
	return ulwip->active;
}

int ulwip_in_loop(void)
{
	struct udevice *udev;
	struct ulwip *ulwip;

	udev = eth_get_dev();
	if (!udev)
		return 0;

	sys_check_timeouts();

	ulwip = eth_lwip_priv(udev);
	return ulwip->loop;
}

void ulwip_loop_set(int loop)
{
	struct udevice *udev;
	struct ulwip *ulwip;

	udev = eth_get_dev();
	if (!udev)
		return;

	ulwip = eth_lwip_priv(udev);
	ulwip->loop = loop;
}

void ulwip_exit(int err)
{
	struct udevice *udev;
	struct ulwip *ulwip;

	udev = eth_get_dev();
	if (!udev)
		return;

	ulwip = eth_lwip_priv(udev);
	ulwip->loop = 0;
	ulwip->err = err;
}

int ulwip_app_get_err(void)
{
	struct udevice *udev;
	struct ulwip *ulwip;

	udev = eth_get_dev();
	if (!udev)
		return 0;

	ulwip = eth_lwip_priv(udev);
	return ulwip->err;
}

typedef struct {
} ulwip_if_t;

static struct pbuf *low_level_input(uchar *data, int len)
{
	struct pbuf *p, *q;

	/* We allocate a pbuf chain of pbufs from the pool. */
	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
	if (!p) {
		LINK_STATS_INC(link.memerr);
		LINK_STATS_INC(link.drop);
		return NULL;
	}

	for (q = p; q != NULL; q = q->next) {
		memcpy(q->payload, data, q->len);
		data += q->len;
	}

	LINK_STATS_INC(link.recv);

	return p;
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

void ulwip_poll(uchar *in_packet, int len)
{
	struct pbuf *p;
	int err;
	struct netif *netif;
	struct udevice *udev;

	udev = eth_get_dev();
	if (!udev)
		return;

	netif = uidx_to_lwip_netif(dev_seq(udev));
	if (!netif)
		return;

	p = low_level_input(in_packet, len);
	if (!p)
		return;

	/* ethernet_input always returns ERR_OK */
	err = ethernet_input(p, netif);
	if (err)
		log_err("ethernet_input err %d\n", err);
}

static int ethernetif_input(struct pbuf *p, struct netif *netif)
{
	return 0;
}

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	int err;
	uchar *pkt = (uchar *)net_tx_packet;

	/* switch dev to active state */
	eth_init_state_only();

	pkt = (uchar *)net_tx_packet;
	memcpy(pkt, p->payload, p->len);

	/* @todo: rewrite DSA code to use pbufs
	 * err = eth_send(p->payload, p->len);
	 */
	err = eth_send(pkt, p->len);
	if (err) {
		log_err("eth_send error %d\n", err);
		pbuf_free(p);
		return ERR_ABRT;
	}
	return ERR_OK;
}

err_t ulwip_if_init(struct netif *netif)
{
	ulwip_if_t *uif;
	struct ulwip *ulwip;

	uif = malloc(sizeof(ulwip_if_t));
	if (!uif) {
		log_err("uif: out of memory\n");
		return ERR_MEM;
	}
	netif->state = uif;

#if defined(CONFIG_LWIP_LIB_DEBUG)
	log_info("              MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
		 netif->hwaddr[0], netif->hwaddr[1], netif->hwaddr[2],
		 netif->hwaddr[3], netif->hwaddr[4], netif->hwaddr[5]);
	log_info("             NAME: %s\n", netif->name);
#endif
#if LWIP_IPV4
	netif->output = etharp_output;
#endif
#if LWIP_IPV6
	netif->output_ip6 = ethip6_output;
#endif

	netif->linkoutput = low_level_output;
	netif->mtu = 1500;
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

	ulwip = eth_lwip_priv(eth_get_dev());
	ulwip->init_done = 1;

	return ERR_OK;
}

static int netif_initialized(char *name)
{
       struct netif *netif;

       NETIF_FOREACH(netif) {
               if (netif->name[0] == name[0] && netif->name[1] == name[1])
                       return 1;
       }

       return 0;
}

int ulwip_init(void)
{
	struct netif *unetif;
	struct ulwip *ulwip;
	struct udevice *udev;
	int ret;

	struct udevice *dev;
	struct uclass *uc;

	eth_set_current();

	udev = eth_get_dev();
	if (!udev) {
		log_err("no active eth device\n");
		return ERR_IF;
	}

	ulwip = eth_lwip_priv(udev);
	if (ulwip->init_done) {
		log_info("init already done for %s\n", udev->name);
		ret = eth_init();
		if (ret)
			return ERR_IF;
		ulwip->active = 1;
		return CMD_RET_SUCCESS;
	}

	eth_init_rings();

	ret = eth_init();
	if (ret) {
		log_err("eth_init error %d\n", ret);
		return ERR_IF;
	}

#ifdef CONFIG_ETH_SANDBOX
	/* switching between flat and live device tree,
	 * clears ulwip->active
	 **/
	if (netif_get_by_index(1)) {
		ulwip->active = 1;
		return CMD_RET_SUCCESS;
	}
#endif

	uclass_id_foreach_dev(UCLASS_ETH, dev, uc) {
		char ipstr[IP4ADDR_STRLEN_MAX];
		char maskstr[IP4ADDR_STRLEN_MAX];
		char gwstr[IP4ADDR_STRLEN_MAX];
		char hwstr[MAC_ADDR_STRLEN];
		unsigned char env_enetaddr[ARP_HLEN];
		char *env, *env_ip;
		struct ulwip *ulwip_priv;
		ip4_addr_t ipaddr, netmask, gw;

		eth_env_get_enetaddr_by_index("eth", dev_seq(dev), env_enetaddr);
		log_info("eth%d: %s %pM %s\n", dev_seq(dev), dev->name, env_enetaddr,
			 udev == dev ? "active" : "");

		unetif = malloc(sizeof(struct netif));
		if (!unetif)
			return ERR_MEM;
		memset(unetif, 0, sizeof(struct netif));

		ip4_addr_set_zero(&gw);
		ip4_addr_set_zero(&ipaddr);
		ip4_addr_set_zero(&netmask);

		if (dev_seq(dev) == 0) {
			snprintf(ipstr, IP4ADDR_STRLEN_MAX, "ipaddr");
			snprintf(maskstr, IP4ADDR_STRLEN_MAX, "netmask");
			snprintf(gwstr, IP4ADDR_STRLEN_MAX, "gw");
		} else {
			snprintf(ipstr, IP4ADDR_STRLEN_MAX, "ipaddr%d", dev_seq(dev));
			snprintf(maskstr, IP4ADDR_STRLEN_MAX, "netmask%d", dev_seq(dev));
			snprintf(gwstr, IP4ADDR_STRLEN_MAX, "gw%d", dev_seq(dev));
		}
		snprintf(hwstr, MAC_ADDR_STRLEN, "%pM",  env_enetaddr);
		snprintf(unetif->name, 2, "%d", dev_seq(dev));

		string_to_enetaddr(hwstr, unetif->hwaddr);
		unetif->hwaddr_len = ETHARP_HWADDR_LEN;

		env_ip = env_get(ipstr);
		if (env_ip)
			ipaddr_aton(env_ip, &ipaddr);

		env = env_get(gwstr);
		if (env)
			ipaddr_aton(env, &gw);

		env = env_get(maskstr);
		if (env)
			ipaddr_aton(env, &netmask);

		if (IS_ENABLED(CONFIG_LWIP_LIB_DEBUG)) {
			log_info("netdev: %s, IP: %s, GW: %s, mask: %s\n",
				 dev->name,
				 ip4addr_ntoa(&ipaddr),
				 ip4addr_ntoa(&gw),
				 ip4addr_ntoa(&netmask));
		}

		if (netif_initialized(unetif->name))
			continue;

		if (!netif_add(unetif, &ipaddr, &netmask, &gw,
			       unetif, ulwip_if_init, ethernetif_input)) {
			log_err("err: netif_add failed!\n");
			free(unetif);
			return ERR_IF;
		}

		if (!netif_is_up(unetif))
			netif_set_up(unetif);
		netif_set_link_up(unetif);

		ulwip_priv = eth_lwip_priv(dev);
		if (!ulwip_priv) {
			log_err("%s() !ulwip_priv for %s\n", __func__, dev->name);
			continue;
		}
		ulwip_priv->active = 1;
	}

	if (IS_ENABLED(CONFIG_LWIP_LIB_DEBUG)) {
		log_info("Initialized LWIP stack\n");
	}

	ulwip->active = 1;
	return CMD_RET_SUCCESS;
}

/* placeholder, not used now */
void ulwip_destroy(void)
{
}
