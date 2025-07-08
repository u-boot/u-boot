// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <console.h>
#include <env.h>
#include <log.h>
#include <dm/device.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <lwip/dhcp.h>
#include <lwip/dns.h>
#include <lwip/timeouts.h>
#include <net.h>
#include <time.h>

#define DHCP_TIMEOUT_MS 10000

#ifdef CONFIG_CMD_TFTPBOOT
/* Boot file obtained from DHCP (if present) */
static char boot_file_name[DHCP_BOOT_FILE_LEN];
#endif

static void call_lwip_dhcp_fine_tmr(void *ctx)
{
	dhcp_fine_tmr();
	sys_timeout(10, call_lwip_dhcp_fine_tmr, NULL);
}

static int dhcp_loop(struct udevice *udev)
{
	char ipstr[] = "ipaddr\0\0";
	char maskstr[] = "netmask\0\0";
	char gwstr[] = "gatewayip\0\0";
	unsigned long start;
	struct netif *netif;
	struct dhcp *dhcp;
	bool bound;
	int idx;

	idx = dev_seq(udev);
	if (idx < 0 || idx > 99) {
		log_err("unexpected idx %d\n", idx);
		return CMD_RET_FAILURE;
	}

	netif = net_lwip_new_netif_noip(udev);
	if (!netif)
		return CMD_RET_FAILURE;

	start = get_timer(0);

	if (dhcp_start(netif))
		return CMD_RET_FAILURE;

	call_lwip_dhcp_fine_tmr(NULL);

	/* Wait for DHCP to complete */
	do {
		net_lwip_rx(udev, netif);
		bound = dhcp_supplied_address(netif);
		if (bound)
			break;
		if (ctrlc()) {
			printf("Abort\n");
			break;
		}
		mdelay(1);
	} while (get_timer(start) < DHCP_TIMEOUT_MS);

	sys_untimeout(call_lwip_dhcp_fine_tmr, NULL);

	if (!bound) {
		net_lwip_remove_netif(netif);
		return CMD_RET_FAILURE;
	}

	dhcp = netif_dhcp_data(netif);

	env_set("bootfile", dhcp->boot_file_name);

	if (idx > 0) {
		sprintf(ipstr, "ipaddr%d", idx);
		sprintf(maskstr, "netmask%d", idx);
		sprintf(gwstr, "gatewayip%d", idx);
	} else {
		net_ip.s_addr = dhcp->offered_ip_addr.addr;
	}

	env_set(ipstr, ip4addr_ntoa(&dhcp->offered_ip_addr));
	env_set(maskstr, ip4addr_ntoa(&dhcp->offered_sn_mask));
	env_set("serverip", ip4addr_ntoa(&dhcp->server_ip_addr));
	if (dhcp->offered_gw_addr.addr != 0)
		env_set(gwstr, ip4addr_ntoa(&dhcp->offered_gw_addr));

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

	net_lwip_remove_netif(netif);
	return CMD_RET_SUCCESS;
}

int do_dhcp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;
	struct udevice *dev;

	if (net_lwip_eth_start() < 0)
		return CMD_RET_FAILURE;

	dev = eth_get_dev();
	if (!dev) {
		log_err("No network device\n");
		return CMD_RET_FAILURE;
	}

	ret = dhcp_loop(dev);
	if (ret)
		return ret;

	if (argc > 1) {
		struct cmd_tbl cmdtp = {};

		return do_tftpb(&cmdtp, 0, argc, argv);
	}

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
