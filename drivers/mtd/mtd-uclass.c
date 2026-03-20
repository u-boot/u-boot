// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 */

#define LOG_CATEGORY UCLASS_MTD

#include <bootdev.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <errno.h>
#include <mtd.h>

static int mtd_post_bind(struct udevice *dev)
{
	if (CONFIG_IS_ENABLED(BOOTDEV_MTD)) {
		int ret;

		ret = bootdev_setup_for_dev(dev, "mtd_bootdev");
		if (ret)
			return log_msg_ret("bd", ret);
	}

	return 0;
}

/*
 * Implement a MTD uclass which should include most flash drivers.
 * The uclass private is pointed to mtd_info.
 */

UCLASS_DRIVER(mtd) = {
	.id		= UCLASS_MTD,
	.name		= "mtd",
	.post_bind	= mtd_post_bind,
	.per_device_auto	= sizeof(struct mtd_info),
};
