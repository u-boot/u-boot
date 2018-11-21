// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <wdt.h>
#include <dm/device-internal.h>
#include <dm/lists.h>

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

static int wdt_post_bind(struct udevice *dev)
{
#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	struct wdt_ops *ops = (struct wdt_ops *)device_get_ops(dev);
	static int reloc_done;

	if (!reloc_done) {
		if (ops->start)
			ops->start += gd->reloc_off;
		if (ops->stop)
			ops->stop += gd->reloc_off;
		if (ops->reset)
			ops->reset += gd->reloc_off;
		if (ops->expire_now)
			ops->expire_now += gd->reloc_off;

		reloc_done++;
	}
#endif
	return 0;
}

UCLASS_DRIVER(wdt) = {
	.id		= UCLASS_WDT,
	.name		= "watchdog",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.post_bind	= wdt_post_bind,
};
