// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <lwip/dhcp.h>
#include <lwip/dns.h>
#include <lwip/timeouts.h>
#include <net-lwip.h>
#include <time.h>

#define DHCP_TIMEOUT_MS 2000

#ifdef CONFIG_CMD_TFTPBOOT
/* Boot file obtained from DHCP (if present) */
static char boot_file_name[DHCP_BOOT_FILE_LEN];
#endif

static void call_lwip_dhcp_fine_tmr(void *ctx)
{
	dhcp_fine_tmr();
}

int do_dhcp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned long start;
	struct netif *netif;
	struct dhcp *dhcp;
	bool bound;

	/* Running DHCP on the primary interface only */
	if (eth_get_dev_index() != 0)
		return -EINVAL;

	net_lwip_init();

	netif = net_lwip_get_netif();
	if (!netif)
		return CMD_RET_FAILURE;

	/*
	 * The fine timer is half a second, it deals with the initial DHCP
	 * request.
	 * We don't bother with the coarse timer (1 minute) which handles the
	 * DHCP lease renewal.
	 */
	sys_timeout(500, call_lwip_dhcp_fine_tmr, NULL);
	start = get_timer(0);
	dhcp_start(netif);

	/* Wait for DHCP to complete */
	do {
		eth_rx();
		sys_check_timeouts();
		bound = dhcp_supplied_address(netif);
		if (bound)
			break;
	} while (get_timer(start) < DHCP_TIMEOUT_MS);

	sys_untimeout(call_lwip_dhcp_fine_tmr, NULL);

	if (!bound)
		return CMD_RET_FAILURE;

	dhcp = netif_dhcp_data(netif);

	env_set("bootfile", dhcp->boot_file_name);
	if (dhcp->offered_gw_addr.addr != 0) {

		env_set("gatewayip", ip4addr_ntoa(&dhcp->offered_gw_addr));
		/* Set this interface as the default for IP routing */
		netif_set_default(netif);
	}
	env_set("ipaddr", ip4addr_ntoa(&dhcp->offered_ip_addr));
	env_set("netmask", ip4addr_ntoa(&dhcp->offered_sn_mask));
	env_set("serverip", ip4addr_ntoa(&dhcp->server_ip_addr));
#ifdef CONFIG_PROT_DNS_LWIP
	env_set("dnsip", ip4addr_ntoa(dns_getserver(0)));
	env_set("dnsip2", ip4addr_ntoa(dns_getserver(1)));
#endif
#ifdef CONFIG_CMD_TFTPBOOT
	if (dhcp->boot_file_name[0] != '\0')
		strncpy(boot_file_name, dhcp->boot_file_name,
			sizeof(boot_file_name));
#endif

	printf("DHCP client bound to address %pI4 (%lu ms)\n",
	       &dhcp->offered_ip_addr, get_timer(start));

	return CMD_RET_SUCCESS;
}

int dhcp_run(ulong addr, const char *fname, bool autoload)
{
	char *dhcp_argv[] = {"dhcp", NULL, };
#ifdef CONFIG_CMD_TFTPBOOT
	char *tftp_argv[] = {"tftpboot", boot_file_name, NULL, };
#endif
	struct cmd_tbl cmdtp = {};	/* dummy */

	if (autoload) {
#ifdef CONFIG_CMD_TFTPBOOT
		/* Assume DHCP was already performed */
		if (boot_file_name[0])
			return do_tftpb(&cmdtp, 0, 2, tftp_argv);
		return 0;
#else
		return -EOPNOTSUPP;
#endif
	}

	return do_dhcp(&cmdtp, 0, 1, dhcp_argv);
}
