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

		if (ip && !ip_addr_isany(ip))
			return true;
	}

	return false;
}

static int sntp_loop(struct net_lwip_ctx *net, ip_addr_t *srvip)
{
	int ret;

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
	for (unsigned int i = 0; i < SNTP_MAX_SERVERS; i++) {
		ip_addr_t addr = *sntp_getserver(i);

		if (ip_addr_isany(&addr))
			continue;
		ret = net_lwip_prepare_addr(net, &addr);
		if (ret)
			return ret;
		sntp_setserver(i, &addr);
	}
	sntp_init();

	sys_timeout(SNTP_TIMEOUT, no_response, NULL);
	while (sntp_state == NOT_DONE) {
		if (net_lwip_poll() < 0)
			break;
		if (ctrlc()) {
			printf("\nAbort\n");
			sntp_state = ABORTED;
			break;
		}
	}
	sys_untimeout(no_response, NULL);

	sntp_stop();

	if (sntp_state == SUCCESS)
		return 0;

	return -1;
}

int do_sntp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct net_lwip_ctx net = {};
	ip_addr_t *srvip = NULL;
	const char *server;
	ip_addr_t ipaddr;
	int ret = CMD_RET_FAILURE;

	switch (argc) {
	case 1:
		server = env_get("ntpserverip");
		break;
	case 2:
		server = argv[1];
		break;
	default:
		return CMD_RET_USAGE;
	}

	if (net_lwip_start(&net, NET_LWIP_ADDR_ENV_STRICT))
		return CMD_RET_FAILURE;
	if (server) {
		if (net_lwip_resolve(&net, server, &ipaddr, IPADDR_TYPE_ANY))
			goto out;
		srvip = &ipaddr;
	}

	if (sntp_loop(&net, srvip) < 0)
		goto out;

	ret = CMD_RET_SUCCESS;

out:
	net_lwip_stop(&net);

	return ret;
}
