// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <malloc.h>
#include <spl.h>

#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

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

	ret = uclass_get_device_by_name(UCLASS_MISC, "usb-hub@2c", &dev);
	if (ret)
		printf("Error bringing up USB hub (%d)\n", ret);

	return 0;
}
