/*
 * Copyright 2017 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <wdt.h>
#include <dm/device-internal.h>
#include <dm/lists.h>

DECLARE_GLOBAL_DATA_PTR;

int wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	const struct wdt_ops *ops = device_get_ops(dev);

	if (!ops->start)
		return -ENOSYS;

	return ops->start(dev, timeout_ms, flags);
}

int wdt_stop(struct udevice *dev)
{
	const struct wdt_ops *ops = device_get_ops(dev);

	if (!ops->stop)
		return -ENOSYS;

	return ops->stop(dev);
}

int wdt_reset(struct udevice *dev)
{
	const struct wdt_ops *ops = device_get_ops(dev);

	if (!ops->reset)
		return -ENOSYS;

	return ops->reset(dev);
}

int wdt_expire_now(struct udevice *dev, ulong flags)
{
	int ret = 0;
	const struct wdt_ops *ops;

	debug("WDT Resetting: %lu\n", flags);
	ops = device_get_ops(dev);
	if (ops->expire_now) {
		return ops->expire_now(dev, flags);
	} else {
		if (!ops->start)
			return -ENOSYS;

		ret = ops->start(dev, 1, flags);
		if (ret < 0)
			return ret;

		hang();
	}

	return ret;
}

UCLASS_DRIVER(wdt) = {
	.id		= UCLASS_WDT,
	.name		= "wdt",
};
