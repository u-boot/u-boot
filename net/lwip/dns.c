// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <console.h>
#include <lwip/dns.h>
#include <lwip/timeouts.h>
#include <net.h>
#include <time.h>

#define DNS_RESEND_MS 1000
#define DNS_TIMEOUT_MS 10000

struct dns_cb_arg {
	ip_addr_t host_ipaddr;
	const char *var;
	bool done;
};

static void do_dns_tmr(void *arg)
{
	dns_tmr();
}

static void dns_cb(const char *name, const ip_addr_t *ipaddr, void *arg)
{
	struct dns_cb_arg *dns_cb_arg = arg;
	char *ipstr = ip4addr_ntoa(ipaddr);

	dns_cb_arg->done = true;

	if (!ipaddr) {
		printf("DNS: host not found\n");
		dns_cb_arg->host_ipaddr.addr = 0;
		return;
	}

	if (dns_cb_arg->var)
		env_set(dns_cb_arg->var, ipstr);

	printf("%s\n", ipstr);
}

static int dns_loop(struct udevice *udev, const char *name, const char *var)
{
	struct dns_cb_arg dns_cb_arg = { };
	bool has_server = false;
	struct netif *netif;
	ip_addr_t ipaddr;
	ip_addr_t ns;
	ulong start;
	char *nsenv;
	int ret;

	dns_cb_arg.var = var;

	netif = net_lwip_new_netif(udev);
	if (!netif)
		return CMD_RET_FAILURE;

	dns_init();

	nsenv = env_get("dnsip");
	if (nsenv && ipaddr_aton(nsenv, &ns)) {
		dns_setserver(0, &ns);
		has_server = true;
	}

	nsenv = env_get("dnsip2");
	if (nsenv && ipaddr_aton(nsenv, &ns)) {
		dns_setserver(1, &ns);
		has_server = true;
	}

	if (!has_server) {
		log_err("No valid name server (dnsip/dnsip2)\n");
		net_lwip_remove_netif(netif);
		return CMD_RET_FAILURE;
	}

	dns_cb_arg.done = false;

	ret = dns_gethostbyname(name, &ipaddr, dns_cb, &dns_cb_arg);

	if (ret == ERR_OK) {
		dns_cb(name, &ipaddr, &dns_cb_arg);
	} else if (ret == ERR_INPROGRESS) {
		start = get_timer(0);
		sys_timeout(DNS_RESEND_MS, do_dns_tmr, NULL);
		do {
			net_lwip_rx(udev, netif);
			if (dns_cb_arg.done)
				break;
			sys_check_timeouts();
			if (ctrlc()) {
				printf("\nAbort\n");
				break;
			}
		} while (get_timer(start) < DNS_TIMEOUT_MS);
		sys_untimeout(do_dns_tmr, NULL);
	}

	net_lwip_remove_netif(netif);

	if (dns_cb_arg.done && dns_cb_arg.host_ipaddr.addr != 0)
		return CMD_RET_SUCCESS;

	return CMD_RET_FAILURE;
}

int do_dns(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char *name;
	char *var = NULL;

	if (argc == 1 || argc > 3)
		return CMD_RET_USAGE;

	name = argv[1];

	if (argc == 3)
		var = argv[2];

	net_lwip_set_current();

	return dns_loop(eth_get_dev(), name, var);
}
