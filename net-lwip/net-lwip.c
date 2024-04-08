// SPDX-License-Identifier: GPL-2.0

/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <lwip/ip4_addr.h>
#include <lwip/err.h>
#include <lwip/netif.h>
#include <lwip/pbuf.h>
#include <lwip/etharp.h>
#include <lwip/prot/etharp.h>
#include <net-lwip.h>

/* xx:xx:xx:xx:xx:xx\0 */
#define MAC_ADDR_STRLEN 18

#if defined(CONFIG_API) || defined(CONFIG_EFI_LOADER)
void (*push_packet)(void *, int len) = 0;
#endif
int net_restart_wrap;
static uchar net_pkt_buf[(PKTBUFSRX+1) * PKTSIZE_ALIGN + PKTALIGN];
uchar *net_rx_packets[PKTBUFSRX];
uchar *net_rx_packet;
uchar *net_tx_packet;

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
        int err;

        /* switch dev to active state */
        eth_init_state_only();

        err = eth_send(p->payload, p->len);
        if (err) {
                log_err("eth_send error %d\n", err);
                return ERR_ABRT;
        }
        return ERR_OK;
}

static err_t net_lwip_if_init(struct netif *netif)
{
#if LWIP_IPV4
	netif->output = etharp_output;
#endif
	netif->linkoutput = low_level_output;
	netif->mtu = 1500;
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

	return ERR_OK;
}

static void eth_init_rings(void)
{
        static bool called;
	int i;

        if (called)
		return;
	called = true;

	net_tx_packet = &net_pkt_buf[0] + (PKTALIGN - 1);
	net_tx_packet -= (ulong)net_tx_packet % PKTALIGN;
	for (i = 0; i < PKTBUFSRX; i++)
		net_rx_packets[i] = net_tx_packet + (i + 1) * PKTSIZE_ALIGN;
}

int net_lwip_init(void)
{
	ip4_addr_t ipaddr, netmask, gw;
	struct netif *unetif;
	struct udevice *udev;
	int ret;
	unsigned char env_enetaddr[ARP_HLEN];
	const struct udevice *dev;
	struct uclass *uc;

	eth_set_current();

	udev = eth_get_dev();
	if (!udev) {
		log_err("no active eth device\n");
		return ERR_IF;
	}

	eth_init_rings();

	ret = eth_init();
	if (ret) {
		log_err("eth_init error %d\n", ret);
		return ERR_IF;
	}

	uclass_id_foreach_dev(UCLASS_ETH, dev, uc) {
		char ipstr[IP4ADDR_STRLEN_MAX];
		char maskstr[IP4ADDR_STRLEN_MAX];
		char gwstr[IP4ADDR_STRLEN_MAX];
		char hwstr[MAC_ADDR_STRLEN];
		char *env;

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

		env = env_get(ipstr);
		if (env)
			ipaddr_aton(env, &ipaddr);

		env = env_get(maskstr);
		if (env)
			ipaddr_aton(env, &netmask);

		if (!netif_add(unetif, &ipaddr, &netmask, &gw,
			       unetif, net_lwip_if_init, netif_input)) {
			log_err("err: netif_add failed!\n");
			free(unetif);
			return ERR_IF;
		}

		netif_set_up(unetif);
		netif_set_link_up(unetif);
	}

	return CMD_RET_SUCCESS;
}

/*
 * Return the current network interface for lwIP. In other words, the struct
 * netif that corresponds to eth_get_dev().
 */
struct netif *net_lwip_get_netif(void)
{
	int idx;

	idx = eth_get_dev_index();
	if (idx < 0)
		return NULL;

	return netif_get_by_index(idx + 1);
}

int net_init(void)
{
	return net_lwip_init();
}

static struct pbuf *alloc_pbuf_and_copy(uchar *data, int len)
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

void net_process_received_packet(uchar *in_packet, int len)
{
	struct netif *netif;
	struct pbuf *pbuf;

	if (len < ETHER_HDR_SIZE)
		return;

#if defined(CONFIG_API) || defined(CONFIG_EFI_LOADER)
	if (push_packet) {
		(*push_packet)(in_packet, len);
		return;
	}
#endif

	netif = net_lwip_get_netif();
	if (!netif)
		return;

	pbuf = alloc_pbuf_and_copy(in_packet, len);
	if (!pbuf)
		return;

	netif->input(pbuf, netif);
}

u32_t sys_now(void)
{
	return get_timer(0);
}
