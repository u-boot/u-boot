// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <console.h>
#include <lwip/dns.h>
#include <lwip/timeouts.h>
#include <net-lwip.h>
#include <time.h>

#define DNS_RESEND_MS 1000
#define DNS_TIMEOUT_MS 10000

static ulong start;
static ip_addr_t host_ipaddr;
static bool done;

static void do_dns_tmr(void *arg)
{
	dns_tmr();
}

static void dns_cb(const char *name, const ip_addr_t *ipaddr, void *arg)
{
	const char *var = arg;
	char *ipstr = ip4addr_ntoa(ipaddr);

	done = true;

	if (!ipaddr) {
		printf("DNS: host not found\n");
		host_ipaddr.addr = 0;
		return;
	}

	if (var)
		env_set(var, ipstr);

	printf("%s\n", ipstr);
}

int do_dns(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	bool has_server = false;
	ip_addr_t ipaddr;
	ip_addr_t ns;
	char *nsenv;
	char *name;
	char *var;
	int ret;

	if (argc == 1 || argc > 3)
		return CMD_RET_USAGE;

	if (argc >= 2)
		name = argv[1];

	if (argc == 3)
		var = argv[2];

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
		return CMD_RET_FAILURE;
	}

	done = false;

	ret = dns_gethostbyname(name, &ipaddr, dns_cb, var);

	if (ret == ERR_OK) {
		dns_cb(name, &ipaddr, var);
	} else if (ret == ERR_INPROGRESS) {
		start = get_timer(0);
		sys_timeout(DNS_RESEND_MS, do_dns_tmr, NULL);
		do {
			eth_rx();
			if (done)
				break;
			sys_check_timeouts();
			if (ctrlc()) {
				printf("\nAbort\n");
				break;
			}
		} while (get_timer(start) < DNS_TIMEOUT_MS);
		sys_untimeout(do_dns_tmr, NULL);
	}

	if (done && host_ipaddr.addr != 0)
		return CMD_RET_SUCCESS;

	return CMD_RET_FAILURE;
}

