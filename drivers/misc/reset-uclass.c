/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <reset.h>
#include <dm.h>
#include <errno.h>
#include <regmap.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <linux/err.h>

int reset_request(struct udevice *dev, enum reset_t type)
{
	struct reset_ops *ops = reset_get_ops(dev);

	if (!ops->request)
		return -ENOSYS;

	return ops->request(dev, type);
}

void reset_walk(enum reset_t type)
{
	struct udevice *dev;
	int ret = 0;

	while (ret != -EINPROGRESS && type < RESET_COUNT) {
		for (uclass_first_device(UCLASS_RESET, &dev);
		dev;
		uclass_next_device(&dev)) {
			ret = reset_request(dev, type);
			if (ret == -EINPROGRESS)
				break;
		}
	}

	/* Wait for the reset to take effect */
	mdelay(100);

	/* Still no reset? Give up */
	printf("Reset not supported on this platform\n");
	hang();
}

/**
 * reset_cpu() - calls reset_walk(RESET_WARM)
 */
void reset_cpu(ulong addr)
{
	reset_walk(RESET_WARM);
}

UCLASS_DRIVER(reset) = {
	.id		= UCLASS_RESET,
	.name		= "reset",
};
