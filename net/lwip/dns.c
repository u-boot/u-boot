// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <console.h>
#include <env.h>
#include <lwip/dns.h>
#include <lwip/nd6.h>
#include <net.h>
#include <time.h>

#define DNS_TIMEOUT_MS 10000

struct dns_result {
	ip_addr_t addr;
	bool done;
};

static bool dns_from_env;

static void dns_clear_env(void)
{
	if (!dns_from_env)
		return;
	for (unsigned int i = 0; i < DNS_MAX_SERVERS; i++)
		dns_setserver(i, NULL);
	dns_from_env = false;
}

int net_lwip_rdnss_accept(struct netif *netif)
{
	static const char *const vars[] = { "dnsip", "dnsip2", "dnsip6", "dnsip6_2" };

	for (unsigned int i = 0; i < ARRAY_SIZE(vars); i++) {
		const char *value = env_get(vars[i]);

		if (value && *value)
			return false;
	}
	/* Withdraw the old override before installing newly learned servers. */
	dns_clear_env();
	return true;
}

static void dns_cb(const char *name, const ip_addr_t *ipaddr, void *arg)
{
	struct dns_result *result = arg;

	ip_addr_set(&result->addr, ipaddr);
	result->done = true;
}

static int dns_server_ready(struct net_lwip_ctx *net, ip_addr_t *server)
{
#if CONFIG_IS_ENABLED(IPV6)
	if (IP_IS_V6(server))
		return net_lwip_ip6_ready(net, server);
#endif
	return ip4_addr_isany_val(*netif_ip4_addr(net->netif)) ? -EAGAIN : 0;
}

/* Install usable environment servers without waiting for another family. */
int net_lwip_dns_refresh(struct net_lwip_ctx *net)
{
	static const char *const v4_first[] = {
		"dnsip", "dnsip2", "dnsip6", "dnsip6_2",
	};
	static const char *const v6_first[] = {
		"dnsip6", "dnsip6_2", "dnsip", "dnsip2",
	};
	const char *const *vars = ip4_addr_isany_val(*netif_ip4_addr(net->netif)) ?
				 v6_first : v4_first;
	ip_addr_t servers[DNS_MAX_SERVERS];
	unsigned int count = 0, configured = 0;
	bool pending = false;
	int ret, error = -EADDRNOTAVAIL;

#if CONFIG_IS_ENABLED(IPV6)
	net_lwip_ip6_update_lifetimes();
#endif

	for (unsigned int i = 0; i < ARRAY_SIZE(v4_first) && count < DNS_MAX_SERVERS; i++) {
		const char *value = env_get(vars[i]);

		if (!value || !*value)
			continue;
		configured++;
		if (!ipaddr_aton(value, &servers[count]) || ip_addr_isany(&servers[count]))
			return -EINVAL;
#if CONFIG_IS_ENABLED(IPV6)
		if (IP_IS_V6(&servers[count]) &&
		    net_lwip_ip6_aton(value, ip_2_ip6(&servers[count])))
			return -EINVAL;
#endif
		ret = dns_server_ready(net, &servers[count]);
		if (ret) {
			pending |= ret == -EAGAIN;
			error = ret;
			continue;
		}
		count++;
	}
	if (!configured) {
		dns_clear_env();
		return -ENOENT;
	}
#if LWIP_ND6_RDNSS_MAX_DNS_SERVERS
	nd6_clear_rdnss(net->netif);
#endif
	for (unsigned int i = 0; i < DNS_MAX_SERVERS; i++)
		dns_setserver(i, i < count ? &servers[i] : NULL);
	dns_from_env = true;
	if (count)
		return 0;
	return pending ? -EAGAIN : error;
}

int net_lwip_dns_init(struct net_lwip_ctx *net)
{
	int ret;

	for (ulong start = get_timer(0); get_timer(start) < 15000;) {
		ret = net_lwip_dns_refresh(net);
		if (!ret)
			return 0;
		if (ret == -ENOENT) {

#if CONFIG_IS_ENABLED(IPV6_ROUTER_DISCOVERY)
			/* RDNSS can configure DNS on an IPv6-only link without DHCP. */
			ret = net_lwip_ip6_init(net);
			if (ret)
				return ret;
			for (unsigned int i = 0; i < DNS_MAX_SERVERS; i++) {
				ip_addr_t server = *dns_getserver(i);

				if (!ip_addr_isany(&server) && !dns_server_ready(net, &server))
					return 0;
			}
#else
			break;
#endif
		} else if (ret != -EAGAIN) {
			return ret;
		}
		if (ctrlc())
			return -EINTR;
		ret = net_lwip_poll();
		if (ret < 0)
			return ret;
	}
	printf("No usable name server configured or advertised\n");
	return -ETIMEDOUT;
}

int net_lwip_dns_lookup(struct net_lwip_ctx *net, const char *name,
			ip_addr_t *addr, u8 type)
{
	struct dns_result result = {};
	u8 dns_type = LWIP_DNS_ADDRTYPE_IPV4;
	ulong start;
	int ret;

#if CONFIG_IS_ENABLED(IPV6)
	if (type == IPADDR_TYPE_V6 ||
	    (type == IPADDR_TYPE_ANY && ip4_addr_isany_val(*netif_ip4_addr(net->netif))))
		dns_type = LWIP_DNS_ADDRTYPE_IPV6;
	else if (type == IPADDR_TYPE_ANY)
		dns_type = LWIP_DNS_ADDRTYPE_IPV4_IPV6;
#endif
	ret = net_lwip_dns_init(net);
	if (ret)
		return ret;
	ret = dns_gethostbyname_addrtype(name, addr, dns_cb, &result, dns_type);
	if (ret == ERR_OK)
		return 0;
	if (ret != ERR_INPROGRESS)
		return -EIO;

	start = get_timer(0);
	ret = -ETIMEDOUT;
	while (get_timer(start) < DNS_TIMEOUT_MS) {
		ret = net_lwip_poll();
		if (ret < 0 || result.done)
			break;
		if (ctrlc()) {
			ret = -EINTR;
			break;
		}
		ret = -ETIMEDOUT;
	}
	dns_cancel(dns_cb, &result);
	if (result.done) {
		if (ip_addr_isany(&result.addr))
			return -ENOENT;
		*addr = result.addr;
		return 0;
	}
	return ret;
}

int do_dns(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct net_lwip_ctx net = {};
	ip_addr_t addr;
	u8 type = IPADDR_TYPE_ANY;
	int ret;

#if CONFIG_IS_ENABLED(IPV6)
	if (argc > 1 && !strcmp(argv[argc - 1], "-ipv6")) {
		type = IPADDR_TYPE_V6;
		argc--;
	}
#endif
	if (argc < 2 || argc > 3)
		return CMD_RET_USAGE;
	if (net_lwip_start(&net, NET_LWIP_ADDR_ENV_STRICT))
		return CMD_RET_FAILURE;
	ret = net_lwip_resolve(&net, argv[1], &addr, type);
	if (!ret) {
		if (argc == 3)
			ret = env_set(argv[2], ipaddr_ntoa(&addr));
		else
			printf("%s\n", ipaddr_ntoa(&addr));
	}
	net_lwip_stop(&net);
	return ret ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}
