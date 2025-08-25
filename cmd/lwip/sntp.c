// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2025 Linaro Ltd. */

#include <command.h>
#include <console.h>
#include <dm/device.h>
#include <env.h>
#include <lwip/apps/sntp.h>
#include <lwip/timeouts.h>
#include <net.h>

U_BOOT_CMD(sntp, 2, 1, do_sntp, "synchronize RTC via network",
	   "[NTPServerNameOrIp]");

#define SNTP_TIMEOUT 10000

static enum done_state {
	NOT_DONE = 0,
	SUCCESS,
	ABORTED,
	TIMED_OUT
} sntp_state;

static void no_response(void *arg)
{
	sntp_state = TIMED_OUT;
}

/* Called by lwIP via the SNTP_SET_SYSTEM_TIME() macro */
void sntp_set_system_time(uint32_t seconds)
{
	char *toff;
	int net_ntp_time_offset = 0;

	toff = env_get("timeoffset");
	if (toff)
		net_ntp_time_offset = simple_strtol(toff, NULL, 10);

	net_sntp_set_rtc(seconds + net_ntp_time_offset);
	sntp_state = SUCCESS;
}

static bool ntp_server_known(void)
{
	int i;

	for (i = 0; i < SNTP_MAX_SERVERS; i++) {
		const ip_addr_t *ip = sntp_getserver(i);

		if (ip && ip->addr)
			return true;
	}

	return false;
}

static int sntp_loop(struct udevice *udev, ip_addr_t *srvip)
{
	struct netif *netif;

	netif = net_lwip_new_netif(udev);
	if (!netif)
		return -1;

	sntp_state = NOT_DONE;

	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_servermode_dhcp(CONFIG_IS_ENABLED(CMD_DHCP));
	if (srvip) {
		sntp_setserver(0, srvip);
	} else {
		if (!ntp_server_known()) {
			log_err("error: ntpserverip not set\n");
			return -1;
		}
	}
	sntp_init();

	sys_timeout(SNTP_TIMEOUT, no_response, NULL);
	while (sntp_state == NOT_DONE) {
		net_lwip_rx(udev, netif);
		if (ctrlc()) {
			printf("\nAbort\n");
			sntp_state = ABORTED;
			break;
		}
	}
	sys_untimeout(no_response, NULL);

	sntp_stop();
	net_lwip_remove_netif(netif);

	if (sntp_state == SUCCESS)
		return 0;

	return -1;
}

int do_sntp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ip_addr_t *srvip;
	char *server;
	ip_addr_t ipaddr;

	switch (argc) {
	case 1:
		srvip = NULL;
		server = env_get("ntpserverip");
		if (server) {
			if (!ipaddr_aton(server, &ipaddr)) {
				printf("ntpserverip is invalid\n");
				return CMD_RET_FAILURE;
			}
			srvip = &ipaddr;
		}
		break;
	case 2:
		if (net_lwip_dns_resolve(argv[1], &ipaddr))
			return CMD_RET_FAILURE;
		srvip = &ipaddr;
		break;
	default:
		return CMD_RET_USAGE;
	}

	if (net_lwip_eth_start() < 0)
		return CMD_RET_FAILURE;

	if (sntp_loop(eth_get_dev(), srvip) < 0)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}
