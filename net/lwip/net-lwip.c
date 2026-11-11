// SPDX-License-Identifier: GPL-2.0

/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <console.h>
#include <env.h>
#include <image.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <hexdump.h>
#include <linux/compiler_attributes.h>
#include <linux/ctype.h>
#include <linux/kernel.h>
#include <lwip/ip4_addr.h>
#include <lwip/dns.h>
#include <lwip/err.h>
#include <lwip/netif.h>
#include <lwip/nd6.h>
#include <lwip/pbuf.h>
#include <lwip/etharp.h>
#include <lwip/ethip6.h>
#include <lwip/ip6.h>
#include <lwip/init.h>
#include <lwip/prot/etharp.h>
#include <lwip/timeouts.h>
#include <lwip-hooks.h>
#include <net.h>
#include <net/pcap.h>
#include <net/wol.h>
#include <timer.h>
#include <time.h>
#include <u-boot/schedule.h>

/* xx:xx:xx:xx:xx:xx\0 */
#define MAC_ADDR_STRLEN 18

#if defined(CONFIG_API) || defined(CONFIG_EFI_LOADER)
void (*push_packet)(void *, int len) = 0;
#endif
int net_try_count;
int net_restart_wrap;
static struct {
	struct udevice *dev;
	struct netif *netif;
	unsigned int users;
	unsigned int env_users;
	unsigned int strict_env_users;
	unsigned int no_addr_users;
	bool polling;
#if CONFIG_IS_ENABLED(IPV6)
	bool ip6_configured;
	u64 ip6_lifetime_time;
	ip6_addr_t ip6_addr;
	ip6_addr_t ip6_gateway;
	u8 ip6_prefix_len;
#endif
} net_lwip_runtime;
static uchar net_pkt_buf[(PKTBUFSRX) * PKTSIZE_ALIGN + PKTALIGN]
	__aligned(PKTALIGN);
const u8 net_bcast_ethaddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
char *pxelinux_configfile;

static err_t net_lwip_tx(struct netif *netif, struct pbuf *p)
{
	struct udevice *udev = netif->state;
	bool pp_allocated = false;
	u32 plen;
	void *pp;
	int err;

	if ((unsigned long)p->payload % PKTALIGN || p->len != p->tot_len) {
		/*
		 * Some net drivers have strict alignment requirements and may
		 * fail or output invalid data if the packet is not aligned.
		 *
		 * A packet may also be stored in multiple chained pbufs. In
		 * this case, assemble the fragments into one contiguous packet
		 * buffer before passing it to the Ethernet driver.
		 */

		pp = memalign(PKTALIGN, p->tot_len);
		if (!pp)
			return ERR_MEM;

		pp_allocated = true;

		plen = pbuf_copy_partial(p, pp, p->tot_len, 0);
		if (plen != p->tot_len) {
			free(pp);
			return ERR_BUF;
		}
	} else {
		pp = p->payload;
		plen = p->len;
	}

	if (CONFIG_IS_ENABLED(LWIP_DEBUG_RXTX)) {
		printf("net_lwip_tx: %u bytes, udev %s\n", plen, udev->name);
		print_hex_dump("net_lwip_tx: ", 0, 16, 1, pp, plen, true);
	}

	err = eth_get_ops(udev)->send(udev, pp, plen);
	if (err >= 0 && CONFIG_IS_ENABLED(CMD_PCAP))
		pcap_post(pp, plen, true);

	if (pp_allocated)
		free(pp);

	if (err) {
		debug("send error %d\n", err);
		return ERR_ABRT;
	}

	return ERR_OK;
}

#if CONFIG_IS_ENABLED(IPV6)
static err_t net_lwip_mld_filter(struct netif *netif, const ip6_addr_t *group,
				 enum netif_mac_filter_action action)
{
	struct udevice *dev = netif->state;
	u8 mac[ARP_HLEN] = { 0x33, 0x33 };

	if (!eth_get_ops(dev)->mcast)
		return ERR_OK;

	memcpy(mac + 2, &group->addr[3], 4);
	return eth_get_ops(dev)->mcast(dev, mac, action == NETIF_ADD_MAC_FILTER) ?
		ERR_IF : ERR_OK;
}
#endif

static err_t net_lwip_if_init(struct netif *netif)
{
#if CONFIG_IS_ENABLED(IPV6)
	ip6_addr_t allnodes;
#endif

	netif->output = etharp_output;
	netif->linkoutput = net_lwip_tx;
	netif->mtu = 1500;
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
#if CONFIG_IS_ENABLED(IPV6)
	netif->output_ip6 = ethip6_output;
	netif->flags |= NETIF_FLAG_MLD6;
	netif_set_mld_mac_filter(netif, net_lwip_mld_filter);
	/* lwIP assumes the all-nodes group is received without an MLD join. */
	ip6_addr_set_allnodes_linklocal(&allnodes);
	return net_lwip_mld_filter(netif, &allnodes, NETIF_ADD_MAC_FILTER);
#endif

	return ERR_OK;
}

static void eth_init_rings(void)
{
	int i;

	for (i = 0; i < PKTBUFSRX; i++)
		net_rx_packets[i] = net_pkt_buf + i  * PKTSIZE_ALIGN;
}

static int get_udev_ipv4_info(struct udevice *dev, ip4_addr_t *ip,
			      ip4_addr_t *mask, ip4_addr_t *gw)
{
	char ipstr[] = "ipaddr\0\0\0";
	char maskstr[] = "netmask\0\0\0";
	char gwstr[] = "gatewayip\0\0\0";
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

#if CONFIG_IS_ENABLED(LOG_SYSLOG)
int net_set_ether(uchar *xet, const uchar *dest_ethaddr, uint prot)
{
	struct ethernet_hdr *et = (struct ethernet_hdr *)xet;
	const uchar *src = eth_get_ethaddr();

	memcpy(et->et_dest, dest_ethaddr, ARP_HLEN);
	if (src)
		memcpy(et->et_src, src, ARP_HLEN);
	else
		memset(et->et_src, 0, ARP_HLEN);
	et->et_protlen = htons(prot);

	return ETHER_HDR_SIZE;
}

static void net_lwip_set_ip_header(uchar *pkt, struct in_addr dest,
				   struct in_addr source, u16 pkt_len,
				   u8 proto)
{
	static u16 ip_id;
	struct ip_udp_hdr *ip = (struct ip_udp_hdr *)pkt;

	ip->ip_hl_v = 0x45;
	ip->ip_tos = 0;
	ip->ip_len = htons(pkt_len);
	ip->ip_p = proto;
	ip->ip_id = htons(ip_id++);
	ip->ip_off = htons(IP_FLAGS_DFRAG);
	ip->ip_ttl = 255;
	ip->ip_sum = 0;
	memcpy(&ip->ip_src, &source, sizeof(source));
	memcpy(&ip->ip_dst, &dest, sizeof(dest));
	ip->ip_sum = compute_ip_checksum(ip, IP_HDR_SIZE);
}

void net_set_udp_header(uchar *pkt, struct in_addr dest, int dport, int sport,
			int len)
{
	struct ip_udp_hdr *ip = (struct ip_udp_hdr *)pkt;
	struct in_addr source = {};
	ip4_addr_t addr, mask, gateway;
	struct udevice *dev = eth_get_dev();
	u16 pkt_len = IP_UDP_HDR_SIZE + len;

	if (len & 1)
		pkt[IP_UDP_HDR_SIZE + len] = 0;

	if (dev && !get_udev_ipv4_info(dev, &addr, &mask, &gateway))
		source.s_addr = addr.addr;

	net_lwip_set_ip_header(pkt, dest, source, pkt_len, IPPROTO_UDP);

	ip->udp_src = htons(sport);
	ip->udp_dst = htons(dport);
	ip->udp_len = htons(UDP_HDR_SIZE + len);
	ip->udp_xsum = 0;
}
#endif

/*
 * Initialize the network stack if needed and start the current device if valid
 */
static int net_lwip_eth_start(void)
{
	int ret;

	net_init();
	eth_halt();
	eth_set_current();
	ret = eth_init();
	if (ret < 0) {
		eth_halt();
		return ret;
	}

	return 0;
}

static void net_lwip_eth_stop(void)
{
	eth_halt();
	if (CONFIG_IS_ENABLED(CMD_PCAP) && pcap_active())
		pcap_print_status();
}

static struct netif *new_netif(struct udevice *udev, bool with_ip)
{
	unsigned char enetaddr[ARP_HLEN];
	char hwstr[MAC_ADDR_STRLEN];
	ip4_addr_t ip, mask, gw;
	struct netif *netif;
	int ret = 0;

	if (!udev)
		return NULL;

	if (eth_start_udev(udev) < 0) {
		log_err("Could not start %s\n", udev->name);
		return NULL;
	}

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

static void net_lwip_remove_netif(struct netif *netif)
{
#if CONFIG_IS_ENABLED(IPV6)
	ip6_addr_t allnodes;
#endif

	netif_remove(netif);
#if CONFIG_IS_ENABLED(DNS)
	/* Do not carry router-advertised name servers to the next interface. */
	for (unsigned int i = 0; i < DNS_MAX_SERVERS; i++)
		dns_setserver(i, NULL);
#endif
#if CONFIG_IS_ENABLED(IPV6)
	ip6_addr_set_allnodes_linklocal(&allnodes);
	net_lwip_mld_filter(netif, &allnodes, NETIF_DEL_MAC_FILTER);
	net_lwip_runtime.ip6_configured = false;
#endif
	free(netif);
}

/* Parse a complete decimal field without overflowing before the range check. */
int net_lwip_parse_decimal(const char *text, size_t len, unsigned int max,
			   unsigned int *value)
{
	unsigned int number = 0;

	if (!len)
		return -EINVAL;
	for (size_t i = 0; i < len; i++) {
		unsigned int digit = (unsigned char)text[i] - '0';

		if (digit > 9 || number > max / 10 ||
		    (number == max / 10 && digit > max % 10))
			return -EINVAL;
		number = number * 10 + digit;
	}
	*value = number;
	return 0;
}

#if CONFIG_IS_ENABLED(IPV6)
/* Age learned configuration without replaying DAD or neighbor probes. */
void net_lwip_ip6_update_lifetimes(void)
{
	u64 elapsed;

	if (!net_lwip_runtime.netif)
		return;
	elapsed = get_timer_us(net_lwip_runtime.ip6_lifetime_time) / 1000000;
	if (!elapsed)
		return;
	/* Keep fractional seconds and advance the clock before expiry callbacks. */
	net_lwip_runtime.ip6_lifetime_time += elapsed * 1000000;
	nd6_age_lifetimes(min_t(u64, elapsed, U32_MAX));
}

int net_lwip_ip6_aton(const char *text, ip6_addr_t *addr)
{
	const char *p = text;
	unsigned int groups = 0;
	bool compressed = false;

	/* Validate groups and compression before using lwIP's permissive parser. */
	if (*p == ':') {
		if (*++p != ':')
			return -EINVAL;
		compressed = true;
		p++;
	}
	while (*p) {
		const char *start = p;

		while (isxdigit(*p))
			p++;
		if (p == start || p - start > 4 || ++groups > 8)
			return -EINVAL;
		if (!*p)
			break;
		if (*p++ != ':')
			return -EINVAL;
		if (*p == ':') {
			if (compressed)
				return -EINVAL;
			compressed = true;
			p++;
		} else if (!*p) {
			return -EINVAL;
		}
	}
	if (compressed ? groups >= 8 : groups != 8)
		return -EINVAL;
	if (!ip6addr_aton(text, addr) || ip6_addr_isany(addr) ||
	    ip6_addr_ismulticast(addr) || ip6_addr_isloopback(addr) ||
	    ip6_addr_isipv4mappedipv6(addr) || ip6_addr_isipv4compat(addr))
		return -EINVAL;

	return 0;
}

static int net_lwip_ip6_env(const char *name, ip6_addr_t *addr, unsigned int *prefix)
{
	const char *value = env_get(name);
	char text[IP6ADDR_STRLEN_MAX];
	char *slash;
	unsigned int bits;

	ip6_addr_set_zero(addr);
	if (prefix)
		*prefix = 64;
	if (!value || !*value)
		return 0;
	if (strlcpy(text, value, sizeof(text)) >= sizeof(text))
		goto invalid;
	slash = strchr(text, '/');
	if (slash) {
		if (!prefix || net_lwip_parse_decimal(slash + 1, strlen(slash + 1),
						      128, &bits))
			goto invalid;
		*prefix = bits;
		*slash = '\0';
	}
	if (net_lwip_ip6_aton(text, addr))
		goto invalid;

	return 0;

invalid:
	printf("Invalid %s: %s\n", name, value);
	return -EINVAL;
}

int net_lwip_ip6_static_prefix(struct netif *netif, unsigned int index,
			       const ip6_addr_t *dest)
{
	const ip6_addr_t *addr = netif_ip6_addr(netif, index);
	const u8 *a = (const u8 *)addr->addr, *b = (const u8 *)dest->addr;
	unsigned int bits = 64, bytes;

	if (netif == net_lwip_runtime.netif && net_lwip_runtime.ip6_configured &&
	    ip6_addr_zoneless_eq(addr, &net_lwip_runtime.ip6_addr))
		bits = net_lwip_runtime.ip6_prefix_len;
	bytes = bits / 8;
	if (memcmp(a, b, bytes))
		return 0;
	return !(bits % 8) || !((a[bytes] ^ b[bytes]) >> (8 - bits % 8));
}

const ip6_addr_t *net_lwip_ip6_gateway(struct netif *netif,
				       const ip6_addr_t *dest)
{
	if (netif != net_lwip_runtime.netif || !net_lwip_runtime.ip6_configured ||
	    ip6_addr_isany(&net_lwip_runtime.ip6_gateway))
		return NULL;

	return &net_lwip_runtime.ip6_gateway;
}

/* Configure IPv6 on demand, without changing a concurrent client's IPv4. */
int net_lwip_ip6_init(struct net_lwip_ctx *ctx)
{
	ip6_addr_t addr, gateway;
	struct netif *netif;
	unsigned int prefix;
	int ret;

	if (!ctx || !ctx->netif || ctx->netif != net_lwip_runtime.netif)
		return -EINVAL;
	net_lwip_ip6_update_lifetimes();
	netif = ctx->netif;
	ret = net_lwip_ip6_env("ip6addr", &addr, &prefix);
	if (ret)
		return ret;
	ret = net_lwip_ip6_env("gatewayip6", &gateway, NULL);
	if (ret)
		return ret;
	if (net_lwip_runtime.ip6_configured) {
		if (prefix != net_lwip_runtime.ip6_prefix_len ||
		    !ip6_addr_zoneless_eq(&addr, &net_lwip_runtime.ip6_addr) ||
		    !ip6_addr_zoneless_eq(&gateway, &net_lwip_runtime.ip6_gateway))
			return -EBUSY;
		return 0;
	}

	if (ip6_addr_islinklocal(&addr)) {
		netif_ip6_addr_set(netif, 0, &addr);
		netif_ip6_addr_set_state(netif, 0, IP6_ADDR_TENTATIVE);
	} else {
		netif_create_ip6_linklocal_address(netif, 1);
		if (!ip6_addr_isany(&addr)) {
			netif_ip6_addr_set(netif, 1, &addr);
			netif_ip6_addr_set_state(netif, 1, IP6_ADDR_TENTATIVE);
		}
	}
#if CONFIG_IS_ENABLED(IPV6_ROUTER_DISCOVERY)
	netif_set_ip6_autoconfig_enabled(netif, 1);
#endif
	ip6_addr_copy(net_lwip_runtime.ip6_addr, addr);
	net_lwip_runtime.ip6_prefix_len = prefix;
	ip6_addr_copy(net_lwip_runtime.ip6_gateway, gateway);
	ip6_addr_assign_zone(&net_lwip_runtime.ip6_gateway, IP6_UNICAST, netif);
	net_lwip_runtime.ip6_configured = true;

	return 0;
}

int net_lwip_ip6_start(struct net_lwip_ctx *ctx)
{
	ulong start;
	int ret = net_lwip_ip6_init(ctx);

	if (ret)
		return ret;
	start = get_timer(0);
	while (!ip6_addr_isvalid(netif_ip6_addr_state(ctx->netif, 0))) {
		if (ip6_addr_isduplicated(netif_ip6_addr_state(ctx->netif, 0)))
			return -EADDRINUSE;
		if (ctrlc())
			return -EINTR;
		if (get_timer(start) >= 15000)
			return -ETIMEDOUT;
		ret = net_lwip_poll();
		if (ret < 0)
			return ret;
	}

	return 0;
}

/* Check readiness without blocking another address family's clients. */
int net_lwip_ip6_ready(struct net_lwip_ctx *ctx, ip_addr_t *dest)
{
	const ip6_addr_t *duplicate = NULL;
	const ip_addr_t *src;
	int ret, i;

	if (!IP_IS_V6(dest) || ip6_addr_isany(ip_2_ip6(dest)) ||
	    ip6_addr_ismulticast(ip_2_ip6(dest)) ||
	    ip6_addr_isloopback(ip_2_ip6(dest)) ||
	    ip6_addr_isipv4mappedipv6(ip_2_ip6(dest)) ||
	    ip6_addr_isipv4compat(ip_2_ip6(dest)))
		return -EINVAL;
	ret = net_lwip_ip6_init(ctx);
	if (ret)
		return ret;
	ip6_addr_assign_zone(ip_2_ip6(dest), IP6_UNICAST, ctx->netif);
	src = ip6_select_source_address(ctx->netif, ip_2_ip6(dest));
	if (src && (ip6_addr_islinklocal(ip_2_ip6(dest)) ||
		    !ip6_addr_islinklocal(ip_2_ip6(src)))) {
		if (ip6_addr_islinklocal(ip_2_ip6(dest)) ||
		    !ip6_addr_isany(&net_lwip_runtime.ip6_gateway) ||
		    nd6_find_route(ip_2_ip6(dest)) == ctx->netif)
			return 0;
		for (i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++) {
			if (ip6_addr_isvalid(netif_ip6_addr_state(ctx->netif, i)) &&
			    netif_ip6_addr_isstatic(ctx->netif, i) &&
			    net_lwip_ip6_static_prefix(ctx->netif, i, ip_2_ip6(dest)))
				return 0;
		}
		return -EAGAIN;
	}
	for (i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++) {
		const ip6_addr_t *addr = netif_ip6_addr(ctx->netif, i);

		if (ip6_addr_islinklocal(addr) != ip6_addr_islinklocal(ip_2_ip6(dest)))
			continue;
		if (ip6_addr_istentative(netif_ip6_addr_state(ctx->netif, i)))
			return -EAGAIN;
		if (ip6_addr_isduplicated(netif_ip6_addr_state(ctx->netif, i)))
			duplicate = addr;
	}
	if (duplicate) {
		printf("Duplicate IPv6 address: %s\n", ip6addr_ntoa(duplicate));
		return -EADDRINUSE;
	}
	return -EAGAIN;
}

/* Wait for DAD and, if needed, an advertised prefix before starting a client. */
int net_lwip_ip6_prepare(struct net_lwip_ctx *ctx, ip_addr_t *dest)
{
	ulong start = get_timer(0);
	int ret;

	do {
		ret = net_lwip_ip6_ready(ctx, dest);
		if (ret != -EAGAIN)
			return ret;
		if (ctrlc())
			return -EINTR;
		ret = net_lwip_poll();
		if (ret < 0)
			return ret;
	} while (get_timer(start) < 15000);

	puts("Timed out waiting for an IPv6 address or route\n");
	return -ETIMEDOUT;
}
#endif

int net_lwip_prepare_addr(struct net_lwip_ctx *ctx, ip_addr_t *addr)
{
#if CONFIG_IS_ENABLED(IPV6)
	if (IP_IS_V6(addr))
		return net_lwip_ip6_prepare(ctx, addr);
#endif
	return 0;
}

static int net_lwip_configure(enum net_lwip_addr_mode addr_mode)
{
	ip4_addr_t ip, mask, gw;

	if (addr_mode != NET_LWIP_ADDR_NONE) {
		if (get_udev_ipv4_info(net_lwip_runtime.dev, &ip, &mask, &gw))
			return -EINVAL;
	} else {
		ip4_addr_set_zero(&ip);
		ip4_addr_set_zero(&mask);
		ip4_addr_set_zero(&gw);
	}

	if (ip4_addr_cmp(netif_ip4_addr(net_lwip_runtime.netif), &ip) &&
	    ip4_addr_cmp(netif_ip4_netmask(net_lwip_runtime.netif), &mask) &&
	    ip4_addr_cmp(netif_ip4_gw(net_lwip_runtime.netif), &gw))
		return 0;

	if (net_lwip_runtime.strict_env_users)
		return -EBUSY;

	netif_set_addr(net_lwip_runtime.netif, &ip, &mask, &gw);

	return 0;
}

static int net_lwip_start_ctx(struct net_lwip_ctx *ctx,
			      enum net_lwip_addr_mode addr_mode,
			      bool eth_started)
{
	struct netif *netif;
	int ret;

	if (!ctx)
		return -EINVAL;
	if (ctx->netif || ctx->dev)
		return -EBUSY;
	if (addr_mode != NET_LWIP_ADDR_ENV_STRICT &&
	    addr_mode != NET_LWIP_ADDR_ENV_FLEXIBLE &&
	    addr_mode != NET_LWIP_ADDR_NONE)
		return -EINVAL;
	if ((addr_mode == NET_LWIP_ADDR_NONE &&
	     (net_lwip_runtime.strict_env_users ||
	      net_lwip_runtime.no_addr_users)) ||
	    (addr_mode == NET_LWIP_ADDR_ENV_STRICT &&
	     net_lwip_runtime.no_addr_users))
		return -EBUSY;

	if (!net_lwip_runtime.users) {
		/* net_start_again() already selected and started the next NIC. */
		if (!eth_started) {
			ret = net_lwip_eth_start();
			if (ret)
				return ret;
		}

		net_lwip_runtime.dev = eth_get_dev();
		netif = new_netif(net_lwip_runtime.dev,
				  addr_mode != NET_LWIP_ADDR_NONE);
		if (!netif) {
			net_lwip_runtime.dev = NULL;
			net_lwip_eth_stop();
			return -ENODEV;
		}
		net_lwip_runtime.netif = netif;
#if CONFIG_IS_ENABLED(IPV6)
		net_lwip_runtime.ip6_lifetime_time = get_timer_us(0);
#endif
	} else if (addr_mode == NET_LWIP_ADDR_NONE &&
		   !net_lwip_runtime.no_addr_users) {
		ret = net_lwip_configure(NET_LWIP_ADDR_NONE);
		if (ret)
			return ret;
	} else if (addr_mode != NET_LWIP_ADDR_NONE &&
		   !net_lwip_runtime.no_addr_users) {
		ret = net_lwip_configure(addr_mode);
		if (ret)
			return ret;
	}

	net_lwip_runtime.users++;
	if (addr_mode == NET_LWIP_ADDR_NONE) {
		net_lwip_runtime.no_addr_users++;
	} else {
		net_lwip_runtime.env_users++;
		if (addr_mode == NET_LWIP_ADDR_ENV_STRICT)
			net_lwip_runtime.strict_env_users++;
	}

	ctx->dev = net_lwip_runtime.dev;
	ctx->netif = net_lwip_runtime.netif;
	ctx->addr_mode = addr_mode;

	return 0;
}

/**
 * net_lwip_start - Attach a client to the shared lwIP runtime
 * @ctx: Zero-initialized client attachment
 * @addr_mode: Initial IPv4 address configuration requested by the client
 *
 * The first client starts the selected Ethernet device and creates the lwIP
 * network interface. Later clients share both resources. Strict environment
 * clients and address-less clients are mutually exclusive. Flexible clients
 * can remain attached while an address-less client temporarily owns the
 * interface configuration.
 *
 * Return: 0 on success, or a negative error code.
 */
int net_lwip_start(struct net_lwip_ctx *ctx,
		   enum net_lwip_addr_mode addr_mode)
{
	return net_lwip_start_ctx(ctx, addr_mode, false);
}

/**
 * net_lwip_stop - Detach a client from the shared lwIP runtime
 * @ctx: Active client attachment
 *
 * The final client removes the lwIP interface and stops Ethernet. When the
 * last address-less client leaves, environment addressing is restored for
 * any clients which remain attached. Callers must first remove every lwIP
 * callback and protocol control block owned by @ctx. This function must not
 * be called from a callback dispatched by net_lwip_poll().
 */
void net_lwip_stop(struct net_lwip_ctx *ctx)
{
	if (!ctx || ctx->netif != net_lwip_runtime.netif ||
	    ctx->dev != net_lwip_runtime.dev || !net_lwip_runtime.users)
		return;

	if (ctx->addr_mode == NET_LWIP_ADDR_NONE) {
		net_lwip_runtime.no_addr_users--;
	} else {
		net_lwip_runtime.env_users--;
		if (ctx->addr_mode == NET_LWIP_ADDR_ENV_STRICT)
			net_lwip_runtime.strict_env_users--;
	}
	net_lwip_runtime.users--;

	ctx->dev = NULL;
	ctx->netif = NULL;

	if (!net_lwip_runtime.users) {
		net_lwip_remove_netif(net_lwip_runtime.netif);
		net_lwip_runtime.netif = NULL;
		net_lwip_runtime.dev = NULL;
		net_lwip_eth_stop();
		return;
	}

	if (!net_lwip_runtime.no_addr_users &&
	    net_lwip_runtime.env_users &&
	    net_lwip_configure(NET_LWIP_ADDR_ENV_FLEXIBLE))
		log_err("Failed to restore lwIP interface addressing\n");
}

/**
 * net_lwip_restart - Restart an exclusively held lwIP runtime
 * @ctx: Active client attachment
 *
 * Stop the current interface, select the next interface according to the
 * normal network retry policy and attach @ctx to the replacement interface.
 * A shared runtime cannot be restarted without disrupting other clients.
 * Wait ten seconds after wrapping around the interfaces, as in legacy.
 * Callers must detach @ctx even on error, including an interrupted wait.
 *
 * Return: 0 on success, -EBUSY if other clients are attached, or another
 * negative error code.
 */
int net_lwip_restart(struct net_lwip_ctx *ctx)
{
	enum net_lwip_addr_mode addr_mode;
	ulong start;
	int ret;

	if (!ctx || ctx->netif != net_lwip_runtime.netif ||
	    ctx->dev != net_lwip_runtime.dev || !net_lwip_runtime.users)
		return -EINVAL;
	if (net_lwip_runtime.users != 1)
		return -EBUSY;

	addr_mode = ctx->addr_mode;
	net_lwip_stop(ctx);

	/* Retain the result for callers such as BOOTP_MAY_FAIL, but do not
	 * carry the previous attempt's wrap into this interface selection.
	 */
	net_restart_wrap = 0;
	ret = net_start_again();
	if (ret)
		return ret;

	ret = net_lwip_start_ctx(ctx, addr_mode, true);
	if (ret)
		return ret;

	if (net_restart_wrap) {
		start = get_timer(0);
		while (get_timer(start) < 10000) {
			if (ctrlc()) {
				puts("\nAbort\n");
				return -EINTR;
			}
			net_lwip_poll();
		}
	}

	return 0;
}

/**
 * net_lwip_refresh - Refresh environment addressing for the shared interface
 * @ctx: Active client attachment
 *
 * The address configuration owner may refresh after publishing its acquired
 * configuration and retiring DHCP/BOOTP, before starting an autoload transfer.
 * Flexible-client refreshes are deferred while that owner remains attached.
 * A refresh which would change the address while a strict client is attached
 * fails with -EBUSY.
 *
 * Return: 0 on success, or a negative error code.
 */
int net_lwip_refresh(struct net_lwip_ctx *ctx)
{
	if (!ctx || ctx->netif != net_lwip_runtime.netif ||
	    ctx->dev != net_lwip_runtime.dev || !net_lwip_runtime.users)
		return -EINVAL;

	if (net_lwip_runtime.no_addr_users && ctx->addr_mode != NET_LWIP_ADDR_NONE)
		return 0;

	return net_lwip_configure(NET_LWIP_ADDR_ENV_FLEXIBLE);
}

/*
 * Initialize the network buffers, an ethernet device, and the lwIP stack
 * (once).
 */
int net_init(void)
{
	static bool init_done;

	if (!init_done) {
		eth_init_rings();
		lwip_init();
		init_done = true;
	}

	return 0;
}

static struct pbuf *alloc_pbuf_and_copy(uchar *data, int len)
{
	struct pbuf *p, *q;

	/* We allocate a pbuf chain of pbufs from the pool. */
	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
	if (!p) {
		debug("Failed to allocate pbuf !!!!!\n");
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

static int net_lwip_rx(struct udevice *udev, struct netif *netif)
{
	struct pbuf *pbuf;
	uchar *packet;
	int flags;
	int len;
	int i;

	/* lwIP timers */
	sys_check_timeouts();
	/* Other tasks and actions */
	schedule();

	if (!eth_is_active(udev))
		return -EINVAL;

	flags = ETH_RECV_CHECK_DEVICE;
	for (i = 0; i < ETH_PACKETS_BATCH_RECV; i++) {
		len = eth_get_ops(udev)->recv(udev, flags, &packet);
		flags = 0;

		if (len > 0) {
			if (CONFIG_IS_ENABLED(CMD_PCAP))
				pcap_post(packet, len, false);

			if (CONFIG_IS_ENABLED(LWIP_DEBUG_RXTX)) {
				printf("net_lwip_tx: %u bytes, udev %s \n", len,
				       udev->name);
				print_hex_dump("net_lwip_rx: ", 0, 16, 1,
					       packet, len, true);
			}

			if (IS_ENABLED(CONFIG_CMD_WOL))
				net_lwip_wol_receive(netif, packet, len);

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

/**
 * net_lwip_poll - Service the shared lwIP runtime
 *
 * Run lwIP timers, schedule other U-Boot work and dispatch received packets
 * to all registered lwIP protocol control blocks. Reentrant calls are rejected
 * so protocol callbacks may safely invoke code which attempts to poll.
 *
 * Return: Receive status, -ENODEV with no active clients, or -EBUSY when a
 * poll is already in progress.
 */
int net_lwip_poll(void)
{
	int ret;

	if (!net_lwip_runtime.users)
		return -ENODEV;
	if (net_lwip_runtime.polling)
		return -EBUSY;

	net_lwip_runtime.polling = true;
#if CONFIG_IS_ENABLED(IPV6)
	net_lwip_ip6_update_lifetimes();
#endif
	ret = net_lwip_rx(net_lwip_runtime.dev, net_lwip_runtime.netif);
	net_lwip_runtime.polling = false;

	return ret;
}

/**
 * net_lwip_dns_resolve() - find IP address from name or IP
 *
 * @name_or_ip: host name or IP address
 * @ip: output IP address
 *
 * Return value: 0 on success, -1 on failure.
 */
int net_lwip_dns_resolve(char *name_or_ip, ip_addr_t *ip)
{
	struct net_lwip_ctx ctx = {};
	int ret;

	ret = net_lwip_start(&ctx, NET_LWIP_ADDR_ENV_STRICT);
	if (!ret)
		ret = net_lwip_resolve(&ctx, name_or_ip, ip, IPADDR_TYPE_ANY);
	net_lwip_stop(&ctx);
	return ret;
}

int net_lwip_resolve(struct net_lwip_ctx *ctx, const char *name,
		     ip_addr_t *addr, u8 type)
{
	if (ipaddr_aton(name, addr)) {
		if (type != IPADDR_TYPE_ANY && IP_GET_TYPE(addr) != type)
			return -EAFNOSUPPORT;
#if CONFIG_IS_ENABLED(IPV6)
		if (IP_IS_V6(addr))
			return net_lwip_ip6_aton(name, ip_2_ip6(addr));
#endif
		return 0;
	}
	/* A malformed address is not a DNS hostname. */
	if (strchr(name, ':'))
		return -EINVAL;
#if CONFIG_IS_ENABLED(DNS)
	return net_lwip_dns_lookup(ctx, name, addr, type);
#else
	return -ENOSYS;
#endif
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
	switch (protocol) {
	case TFTPGET:
		/* Match the legacy loop, not the shell command's return value.
		 * In particular, bubt treats a positive result as downloaded data.
		 * Its caller supplies the load address and filename directly.
		 */
		net_boot_file_size = 0;
		if (tftpb_run(image_load_addr, net_boot_file_name))
			return -EINVAL;
		return !!net_boot_file_size;
	default:
		return -EINVAL;
	}

	return -EINVAL;
}

u32_t sys_now(void)
{
#if CONFIG_IS_ENABLED(SANDBOX_TIMER)
	return timer_early_get_count() / 1000;
#else
	return get_timer(0);
#endif
}

int net_start_again(void)
{
	char *nretry;
	int retry_forever = 0;
	unsigned long retrycnt = 0;

	nretry = env_get("netretry");
	if (nretry) {
		if (!strcmp(nretry, "yes"))
			retry_forever = 1;
		else if (!strcmp(nretry, "no"))
			retrycnt = 0;
		else if (!strcmp(nretry, "once"))
			retrycnt = 1;
		else
			retrycnt = simple_strtoul(nretry, NULL, 0);
	} else {
		retrycnt = 0;
		retry_forever = 0;
	}

	if ((!retry_forever) && (net_try_count > retrycnt)) {
		eth_halt();
		/*
		 * We don't provide a way for the protocol to return an error,
		 * but this is almost always the reason.
		 */
		return -ETIMEDOUT;
	}

	eth_halt();
#if !defined(CONFIG_NET_DO_NOT_TRY_ANOTHER)
	eth_try_another(net_try_count == 1);
#endif
	net_try_count++;
	return eth_init();
}
