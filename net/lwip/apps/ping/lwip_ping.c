// SPDX-License-Identifier: GPL-2.0

/*
 * (C) Copyright 2023 Linaro Ltd. <maxim.uvarov@linaro.org>
 */

#include "lwip/opt.h"
#include "lwip/ip_addr.h"
#include "lwip/timeouts.h"
#include <linux/errno.h>
#include "ping.h"
#include "lwip_ping.h"

#define PING_WAIT_MS 5000

static ip_addr_t ip_target;

void ping_tmo(void *arg)
{
	log_err("%s: ping failed; host %s is not alive\n",
		__func__, ipaddr_ntoa(&ip_target));
	ulwip_exit(1);
}

int ulwip_ping(char *ping_addr)
{
	int err;

	err = ipaddr_aton(ping_addr, &ip_target);
	if (!err)
		return -ENOENT;

	sys_timeout(PING_WAIT_MS, ping_tmo, NULL);

	ping_init(&ip_target);
	ping_send_now();

	return 0;
}
