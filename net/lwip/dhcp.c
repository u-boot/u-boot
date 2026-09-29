// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2024 Linaro Ltd. */

#include <command.h>
#include <console.h>
#include <env.h>
#include <log.h>
#include <dm/device.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <lwip/apps/sntp.h>
#include <lwip/dhcp.h>
#include <lwip/dns.h>
#include <net.h>
#include <time.h>

#define DHCP_TIMEOUT_MS 10000

static int dhcp_loop(struct net_lwip_ctx *net, bool explicit_file)
{
	char ipstr[] = "ipaddr\0\0\0";
	char maskstr[] = "netmask\0\0\0";
	char gwstr[] = "gatewayip\0\0\0";
	const ip_addr_t *ntpserverip;
	unsigned long start;
	struct dhcp *dhcp;
	bool bound = false;
	int idx;

	idx = dev_seq(net->dev);
	if (idx < 0 || idx > 99) {
		log_err("unexpected idx %d\n", idx);
		return CMD_RET_FAILURE;
	}

	/*
	 * Request the DHCP stack to parse and store the NTP servers for
	 * eventual use by the SNTP command
	 */
	if (CONFIG_IS_ENABLED(CMD_SNTP))
		sntp_servermode_dhcp(1);

	start = get_timer(0);

	if (dhcp_start(net->netif))
		return CMD_RET_FAILURE;

	/* Wait for DHCP to complete */
	do {
		if (net_lwip_poll() < 0)
			return CMD_RET_FAILURE;
		bound = dhcp_supplied_address(net->netif);
		if (bound)
			break;
		if (ctrlc()) {
			printf("Abort\n");
			break;
		}
		mdelay(1);
	} while (get_timer(start) < DHCP_TIMEOUT_MS);

	if (!bound)
		return CMD_RET_FAILURE;

	dhcp = netif_dhcp_data(net->netif);

	if (!explicit_file && dhcp->boot_file_name[0])
		copy_filename(net_boot_file_name, dhcp->boot_file_name,
			      sizeof(net_boot_file_name));
	if (*net_boot_file_name && env_set("bootfile", net_boot_file_name))
		return CMD_RET_FAILURE;

	if (idx > 0) {
		sprintf(ipstr, "ipaddr%d", idx);
		sprintf(maskstr, "netmask%d", idx);
		sprintf(gwstr, "gatewayip%d", idx);
	} else {
		net_ip.s_addr = ip_addr_get_ip4_u32(&dhcp->offered_ip_addr);
	}

	if (env_set(ipstr, ip4addr_ntoa(&dhcp->offered_ip_addr)) ||
	    env_set(maskstr, ip4addr_ntoa(&dhcp->offered_sn_mask)) ||
	    env_set("serverip", ip4addr_ntoa(&dhcp->server_ip_addr)) ||
	    env_set(gwstr, ip4addr_ntoa(&dhcp->offered_gw_addr)) ||
	    env_set("tftpserverip", ip4_addr_isany(&dhcp->offered_si_addr) ?
		    NULL : ip4addr_ntoa(&dhcp->offered_si_addr)))
		return CMD_RET_FAILURE;

#ifdef CONFIG_PROT_DNS_LWIP
	if (env_set("dnsip", ip4addr_ntoa(dns_getserver(0))) ||
	    env_set("dnsip2", ip4addr_ntoa(dns_getserver(1))))
		return CMD_RET_FAILURE;
#endif
	if (CONFIG_IS_ENABLED(CMD_SNTP)) {
		ntpserverip = sntp_getserver(1);
		if (ntpserverip != IP_ADDR_ANY)
			env_set("ntpserverip", ip4addr_ntoa(ntpserverip));
	}

	printf("DHCP client bound to address %pI4 (%lu ms)\n",
	       &dhcp->offered_ip_addr, get_timer(start));

	return CMD_RET_SUCCESS;
}

static int dhcp_nfs_autoload(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[])
{
	const char *server = env_get("tftpserverip");
	int ret;

	/*
	 * DHCP's siaddr is a boot server, not necessarily the address server
	 * stored in serverip. Supply it only for this download, preserving an
	 * explicit nfsserverip and the NFS command's filename-prefix priority.
	 */
	if (!server || env_get("nfsserverip"))
		return do_nfs(cmdtp, flag, argc, argv);
	if (env_set("nfsserverip", server))
		return CMD_RET_FAILURE;
	ret = do_nfs(cmdtp, flag, argc, argv);
	if (env_set("nfsserverip", NULL))
		return CMD_RET_FAILURE;

	return ret;
}

int do_dhcp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct net_lwip_ctx net = {};
	const char *autoload;
	const char *filename = NULL;
	ulong addr;
	char *end;
	int ret;

	switch (argc) {
	case 1:
		break;
	case 2:
		hextoul(argv[1], &end);
		if (end == argv[1] || *end)
			filename = argv[1];
		break;
	case 3:
		if (strict_strtoul(argv[1], 16, &addr))
			return CMD_RET_USAGE;
		filename = argv[2];
		break;
	default:
		return CMD_RET_USAGE;
	}
	copy_filename(net_boot_file_name, filename ?: env_get("bootfile"),
		      sizeof(net_boot_file_name));

	if (net_lwip_start(&net, NET_LWIP_ADDR_NONE))
		return CMD_RET_FAILURE;

	ret = dhcp_loop(&net, !!filename);
	/*
	 * A download needs strict addressing, incompatible with DHCP's
	 * address-less runtime attachment. Retire DHCP before handing off.
	 */
	if (net.netif) {
		if (dhcp_supplied_address(net.netif))
			dhcp_stop_without_release(net.netif);
		else
			dhcp_release_and_stop(net.netif);
		dhcp_cleanup(net.netif);
	}
	net_lwip_stop(&net);

	if (ret)
		return ret;
	autoload = env_get("autoload");
	if (autoload && !strcmp(autoload, "NFS")) {
		if (IS_ENABLED(CONFIG_CMD_NFS))
			return dhcp_nfs_autoload(cmdtp, flag, argc, argv);
		log_err("Cannot autoload with NFS: command is disabled\n");
		return CMD_RET_FAILURE;
	}
	if (env_get_yesno("autoload") == 0)
		return CMD_RET_SUCCESS;
	if (IS_ENABLED(CONFIG_CMD_TFTPBOOT))
		return do_tftpb(cmdtp, flag, argc, argv);
	log_err("Cannot autoload with TFTP: command is disabled\n");

	return CMD_RET_FAILURE;
}
