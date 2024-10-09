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
#include <lwip/init.h>
#include <lwip/prot/etharp.h>
#include <net.h>

/* xx:xx:xx:xx:xx:xx\0 */
#define MAC_ADDR_STRLEN 18

#if defined(CONFIG_API) || defined(CONFIG_EFI_LOADER)
void (*push_packet)(void *, int len) = 0;
#endif
int net_restart_wrap;
static uchar net_pkt_buf[(PKTBUFSRX) * PKTSIZE_ALIGN + PKTALIGN];
uchar *net_rx_packets[PKTBUFSRX];
uchar *net_rx_packet;
const u8 net_bcast_ethaddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
char *pxelinux_configfile;
/* Our IP addr (0 = unknown) */
struct in_addr	net_ip;
char net_boot_file_name[1024];

static err_t linkoutput(struct netif *netif, struct pbuf *p)
{
	struct udevice *udev = netif->state;
	void *pp = NULL;
	int err;

	if ((unsigned long)p->payload % PKTALIGN) {
		/*
		 * Some net drivers have strict alignment requirements and may
		 * fail or output invalid data if the packet is not aligned.
		 */
		pp = memalign(PKTALIGN, p->len);
		if (!pp)
			return ERR_ABRT;
		memcpy(pp, p->payload, p->len);
	}

	err = eth_get_ops(udev)->send(udev, pp ? pp : p->payload, p->len);
	free(pp);
	if (err) {
		log_err("send error %d\n", err);
		return ERR_ABRT;
	}

	return ERR_OK;
}

static err_t net_lwip_if_init(struct netif *netif)
{
	netif->output = etharp_output;
	netif->linkoutput = linkoutput;
	netif->mtu = 1500;
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

	return ERR_OK;
}

static void eth_init_rings(void)
{
	int i;

	for (i = 0; i < PKTBUFSRX; i++)
		net_rx_packets[i] = net_pkt_buf + i  * PKTSIZE_ALIGN;
}

struct netif *net_lwip_get_netif(void)
{
	struct netif *netif, *found = NULL;

	NETIF_FOREACH(netif) {
		if (!found)
			found = netif;
		else
			printf("Error: more than one netif in lwIP\n");
	}
	return found;
}

static int get_udev_ipv4_info(struct udevice *dev, ip4_addr_t *ip,
			      ip4_addr_t *mask, ip4_addr_t *gw)
{
	char *ipstr = "ipaddr\0\0";
	char *maskstr = "netmask\0\0";
	char *gwstr = "gatewayip\0\0";
	int idx = dev_seq(dev);
	char *env;

	if (idx < 0 || idx > 99) {
		log_err("unexpected idx %d\n", idx);
		return -1;
	}

	if (idx) {
		sprintf(ipstr, "ipaddr%d", idx);
		sprintf(maskstr, "netmask%d", idx);
		sprintf(gwstr, "gatewayip%d", idx);
	}

	ip4_addr_set_zero(ip);
	ip4_addr_set_zero(mask);
	ip4_addr_set_zero(gw);

	env = env_get(ipstr);
	if (env)
		ip4addr_aton(env, ip);

	env = env_get(maskstr);
	if (env)
		ip4addr_aton(env, mask);

	env = env_get(gwstr);
	if (env)
		ip4addr_aton(env, gw);

	return 0;
}

static struct netif *new_netif(struct udevice *udev, bool with_ip)
{
	unsigned char enetaddr[ARP_HLEN];
	char hwstr[MAC_ADDR_STRLEN];
	ip4_addr_t ip, mask, gw;
	struct netif *netif;
	int ret = 0;
	static bool first_call = true;

	if (!udev)
		return NULL;

	if (first_call) {
		eth_init_rings();
		/* Pick a valid active device, if any */
		eth_init();
		lwip_init();
		first_call = false;
	}

	if (eth_start_udev(udev) < 0) {
		log_err("Could not start %s\n", udev->name);
		return NULL;
	}

	netif_remove(net_lwip_get_netif());

	ip4_addr_set_zero(&ip);
	ip4_addr_set_zero(&mask);
	ip4_addr_set_zero(&gw);

	if (with_ip)
		if (get_udev_ipv4_info(udev, &ip, &mask, &gw) < 0)
			return NULL;

	eth_env_get_enetaddr_by_index("eth", dev_seq(udev), enetaddr);
	ret = snprintf(hwstr, MAC_ADDR_STRLEN, "%pM",  enetaddr);
	if (ret < 0 || ret >= MAC_ADDR_STRLEN)
		return NULL;

	netif = calloc(1, sizeof(struct netif));
	if (!netif)
		return NULL;

	netif->name[0] = 'e';
	netif->name[1] = 't';

	string_to_enetaddr(hwstr, netif->hwaddr);
	netif->hwaddr_len = ETHARP_HWADDR_LEN;
	debug("adding lwIP netif for %s with hwaddr:%s ip:%s ", udev->name,
	      hwstr, ip4addr_ntoa(&ip));
	debug("mask:%s ", ip4addr_ntoa(&mask));
	debug("gw:%s\n", ip4addr_ntoa(&gw));

	if (!netif_add(netif, &ip, &mask, &gw, udev, net_lwip_if_init,
		       netif_input)) {
		printf("error: netif_add() failed\n");
		free(netif);
		return NULL;
	}

	netif_set_up(netif);
	netif_set_link_up(netif);
	/* Routing: use this interface to reach the default gateway */
	netif_set_default(netif);

	return netif;
}

struct netif *net_lwip_new_netif(struct udevice *udev)
{
	return new_netif(udev, true);
}

struct netif *net_lwip_new_netif_noip(struct udevice *udev)
{

	return new_netif(udev, false);
}

void net_lwip_remove_netif(struct netif *netif)
{
	netif_remove(netif);
	free(netif);
}

int net_init(void)
{
	eth_set_current();

	net_lwip_new_netif(eth_get_dev());

	return 0;
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

int net_lwip_rx(struct udevice *udev, struct netif *netif)
{
	struct pbuf *pbuf;
	uchar *packet;
	int flags;
	int len;
	int i;

	if (!eth_is_active(udev))
		return -EINVAL;

	flags = ETH_RECV_CHECK_DEVICE;
	for (i = 0; i < ETH_PACKETS_BATCH_RECV; i++) {
		len = eth_get_ops(udev)->recv(udev, flags, &packet);
		flags = 0;

		if (len > 0) {
			pbuf = alloc_pbuf_and_copy(packet, len);
			if (pbuf)
				netif->input(pbuf, netif);
		}
		if (len >= 0 && eth_get_ops(udev)->free_pkt)
			eth_get_ops(udev)->free_pkt(udev, packet, len);
		if (len <= 0)
			break;
	}
	if (len == -EAGAIN)
		len = 0;

	return len;
}

void net_process_received_packet(uchar *in_packet, int len)
{
#if defined(CONFIG_API) || defined(CONFIG_EFI_LOADER)
	if (push_packet)
		(*push_packet)(in_packet, len);
#endif
}

int net_loop(enum proto_t protocol)
{
	char *argv[1];

	switch (protocol) {
	case TFTPGET:
		argv[0] = "tftpboot";
		return do_tftpb(NULL, 0, 1, argv);
	default:
		return -EINVAL;
	}

	return -EINVAL;
}

u32_t sys_now(void)
{
	return get_timer(0);
}
