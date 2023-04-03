// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <env.h>
#include <env_internal.h>
#include <malloc.h>
#include <net.h>
#include <spl.h>

#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

static void dmo_setup_second_mac_address(void)
{
	u8 enetaddr[6];
	int ret;

	/* In case 'eth1addr' is already set in environment, do nothing. */
	ret = eth_env_get_enetaddr_by_index("eth", 1, enetaddr);
	if (ret)	/* valid 'eth1addr' is already set */
		return;

	/* Read 'ethaddr' from environment and validate. */
	ret = eth_env_get_enetaddr_by_index("eth", 0, enetaddr);
	if (!ret)	/* 'ethaddr' in environment is not valid, stop */
		return;

	/* Set 'eth1addr' as 'ethaddr' + 1 */
	enetaddr[5]++;

	eth_env_set_enetaddr_by_index("eth", 1, enetaddr);
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	/* Environment is always in eMMC boot partitions */
	return prio ? ENVL_UNKNOWN : ENVL_MMC;
}

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	struct udevice *dev;
	int ret;

	dmo_setup_boot_device();
	dmo_setup_mac_address();
	dmo_setup_second_mac_address();

	ret = uclass_get_device_by_name(UCLASS_MISC, "usb-hub@2c", &dev);
	if (ret)
		printf("Error bringing up USB hub (%d)\n", ret);

	return 0;
}
