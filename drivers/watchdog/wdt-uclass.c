// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 Google, Inc
 */

#define LOG_CATEGORY UCLASS_WDT

#include <cyclic.h>
#include <div64.h>
#include <dm.h>
#include <errno.h>
#include <hang.h>
#include <log.h>
#include <sysreset.h>
#include <time.h>
#include <wdt.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <linux/kernel.h>

#define WATCHDOG_TIMEOUT_SECS	(CONFIG_WATCHDOG_TIMEOUT_MSECS / 1000)

struct wdt_priv {
	/* The udevice owning this wdt_priv. */
	struct udevice *dev;
	/* Timeout, in seconds, to configure this device to. */
	u32 timeout;
	/*
	 * Time, in milliseconds, between calling the device's ->reset()
	 * method from schedule().
	 */
	ulong reset_period;
	/*
	 * Next time (as returned by get_timer(0)) to call
	 * ->reset().
	 */
	ulong next_reset;
	/* Whether watchdog_start() has been called on the device. */
	bool running;
	/* autostart */
	bool autostart;

	struct cyclic_info cyclic;
};

int wdt_set_force_autostart(struct udevice *dev)
{
	struct wdt_priv *priv = dev_get_uclass_priv(dev);

	priv->autostart = true;

	return 0;
}

static void wdt_cyclic(struct cyclic_info *c)
{
	struct wdt_priv *priv = container_of(c, struct wdt_priv, cyclic);
	struct udevice *dev = priv->dev;

	if (!device_active(dev))
		return;

	if (!priv->running)
		return;

	wdt_reset(dev);
}

static void init_watchdog_dev(struct udevice *dev)
{
	struct wdt_priv *priv;
	int ret;

	priv = dev_get_uclass_priv(dev);

	if (IS_ENABLED(CONFIG_SYSRESET_WATCHDOG_AUTO)) {
		ret = sysreset_register_wdt(dev);
		if (ret)
			printf("WDT:   Failed to register %s for sysreset\n",
			       dev->name);
	}

	if (!priv->autostart) {
		printf("WDT:   Not starting %s\n", dev->name);
		return;
	}

	ret = wdt_start(dev, priv->timeout * 1000, 0);
	if (ret != 0) {
		printf("WDT:   Failed to start %s\n", dev->name);
		return;
	}
}

int initr_watchdog(void)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_WDT, &uc);
	if (ret) {
		log_debug("Error getting UCLASS_WDT: %d\n", ret);
		return 0;
	}

	uclass_foreach_dev(dev, uc) {
		ret = device_probe(dev);
		if (ret) {
			log_debug("Error probing %s: %d\n", dev->name, ret);
			continue;
		}
		init_watchdog_dev(dev);
	}

	return 0;
}

int wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	const struct wdt_ops *ops = device_get_ops(dev);
	struct wdt_uc_plat *plat = dev_get_uclass_plat(dev);
	u64 req_timeout_ms = timeout_ms;
	int ret;

	if (!ops->start)
		return -ENOSYS;
	/* Clamp to max timeout if reported by driver */
	if (plat->max_timeout_ms && timeout_ms > plat->max_timeout_ms)
		timeout_ms = plat->max_timeout_ms;

	ret = ops->start(dev, timeout_ms, flags);
	if (ret == 0) {
		struct wdt_priv *priv = dev_get_uclass_priv(dev);
		char svc_str[16];
		char req_str[32];
		u32 req_s = lldiv(req_timeout_ms, 1000);
		u32 tout_s = lldiv(timeout_ms, 1000);

		svc_str[0] = '\0';
		if (IS_ENABLED(CONFIG_WATCHDOG)) {
			if (priv->running)
				cyclic_unregister(&priv->cyclic);

			/* Register the watchdog driver as a cyclic function */
			cyclic_register(&priv->cyclic, wdt_cyclic,
					priv->reset_period * 1000,
					dev->name);

			snprintf(svc_str, sizeof(svc_str), "every %ldms",
				 priv->reset_period);
		}

		priv->running = true;

		/*
		 * If the requested timeout was clamped, note the value
		 * when it differs at whole-second resolution. Sub-second
		 * rounding is ignored to avoid noise.
		 */
		req_str[0] = '\0';
		if (req_s != tout_s)
			snprintf(req_str, sizeof(req_str), ", requested %ds", req_s);

		printf("WDT:   Started %s with%s servicing %s (%ds timeout%s)\n",
		       dev->name, IS_ENABLED(CONFIG_WATCHDOG) ? "" : "out",
		       svc_str, tout_s, req_str);
	}

	return ret;
}

int wdt_stop(struct udevice *dev)
{
	const struct wdt_ops *ops = device_get_ops(dev);
	int ret;

	if (!ops->stop)
		return -ENOSYS;

	ret = ops->stop(dev);
	if (ret == 0) {
		struct wdt_priv *priv = dev_get_uclass_priv(dev);

		if (IS_ENABLED(CONFIG_WATCHDOG) && priv->running)
			cyclic_unregister(&priv->cyclic);

		priv->running = false;
	}

	return ret;
}

int wdt_stop_all(void)
{
	struct wdt_priv *priv;
	struct udevice *dev;
	struct uclass *uc;
	int ret, err;

	ret = uclass_get(UCLASS_WDT, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		if (!device_active(dev))
			continue;
		priv = dev_get_uclass_priv(dev);
		if (!priv->running)
			continue;
		err = wdt_stop(dev);
		if (!ret)
			ret = err;
	}

	return ret;
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
		ret = wdt_start(dev, 1, flags);

		if (ret < 0)
			return ret;

		hang();
	}

	return ret;
}

static int wdt_pre_probe(struct udevice *dev)
{
	u32 timeout = WATCHDOG_TIMEOUT_SECS;
	/*
	 * Reset every 1000ms, or however often is required as
	 * indicated by a hw_margin_ms property.
	 */
	ulong reset_period = 1000;
	bool autostart = IS_ENABLED(CONFIG_WATCHDOG_AUTOSTART);
	struct wdt_priv *priv;

	if (CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)) {
		timeout = dev_read_u32_default(dev, "timeout-sec", timeout);
		reset_period = dev_read_u32_default(dev, "hw_margin_ms",
						    4 * reset_period) / 4;
		if (dev_read_bool(dev, "u-boot,noautostart"))
			autostart = false;
		else if (dev_read_bool(dev, "u-boot,autostart"))
			autostart = true;
	}
	priv = dev_get_uclass_priv(dev);
	priv->dev = dev;
	priv->timeout = timeout;
	priv->reset_period = reset_period;
	priv->autostart = autostart;
	/*
	 * Pretend this device was last reset "long" ago so the first
	 * schedule() will actually call its ->reset method.
	 */
	priv->next_reset = get_timer(0);

	return 0;
}

/*
 * The cyclic_info that wdt_start() registered lives inside this device's uclass
 * private data, which device_free() releases as soon as the device is removed.
 * Take it off the cyclic list first, or the next schedule() walks a freed node
 * and calls through whatever has since been allocated over it.
 */
static int wdt_pre_remove(struct udevice *dev)
{
	struct wdt_priv *priv = dev_get_uclass_priv(dev);

	if (!IS_ENABLED(CONFIG_WATCHDOG) || !priv || !priv->running)
		return 0;

	cyclic_unregister(&priv->cyclic);
	priv->running = false;

	return 0;
}

UCLASS_DRIVER(wdt) = {
	.id			= UCLASS_WDT,
	.name			= "watchdog",
	.flags			= DM_UC_FLAG_SEQ_ALIAS,
	.pre_probe		= wdt_pre_probe,
	.pre_remove		= wdt_pre_remove,
	.per_device_auto	= sizeof(struct wdt_priv),
	.per_device_plat_auto	= sizeof(struct wdt_uc_plat),
};
