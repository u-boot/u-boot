// SPDX-License-Identifier: GPL-2.0

/*
 * (C) Copyright 2023 Linaro Ltd. <maxim.uvarov@linaro.org>
 */

#include <command.h>
#include <net.h>
#include "net/lwip.h"
#include "net/ulwip.h"

int ulwip_loop(void)
{
	int ret = CMD_RET_FAILURE;
	struct udevice *udev;
	struct ulwip *ulwip;

	udev = eth_get_dev();
	if (!udev)
		return -1;

	ulwip = eth_lwip_priv(udev);

	ulwip_loop_set(1);
	if (!net_loop(LWIP))
		ret = CMD_RET_SUCCESS;
	ulwip_loop_set(0);
	net_loop(LWIP);
	ulwip->active = 0;
	return ret;
}
