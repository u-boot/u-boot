// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on the Linux drivers/watchdog/da9063_wdt.c file.
 *
 * Watchdog driver for DA9063 PMICs.
 *
 * Copyright(c) 2012 Dialog Semiconductor Ltd.
 *
 * Author: Mariusz Wojtasik <mariusz.wojtasik@diasemi.com>
 *
 * Ported to U-Boot by Fabio Estevam <festevam@denx.de>
 *
 */

#include <dm.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <i2c.h>
#include <linux/delay.h>
#include <wdt.h>

#define	DA9063_REG_CONTROL_D		0x11
/* DA9063_REG_CONTROL_D (addr=0x11) */
#define	DA9063_TWDSCALE_MASK		0x0
#define DA9063_TWDSCALE_DISABLE		0
#define	DA9063_REG_CONTROL_F		0x13
/* DA9063_REG_CONTROL_F (addr=0x13) */
#define	DA9063_WATCHDOG			0x01
#define	DA9063_SHUTDOWN			0x02

/*
 * Watchdog selector to timeout in seconds.
 *   0: WDT disabled;
 *   others: timeout = 2048 ms * 2^(TWDSCALE-1).
 */
static const unsigned int wdt_timeout[] = { 0, 2, 4, 8, 16, 32, 65, 131 };

#define DA9063_TWDSCALE_DISABLE		0
#define DA9063_TWDSCALE_MIN		1
#define DA9063_TWDSCALE_MAX		(ARRAY_SIZE(wdt_timeout) - 1)

static unsigned int da9063_wdt_timeout_to_sel(unsigned int secs)
{
	unsigned int i;

	for (i = DA9063_TWDSCALE_MIN; i <= DA9063_TWDSCALE_MAX; i++) {
		if (wdt_timeout[i] >= secs)
			return i;
	}

	return DA9063_TWDSCALE_MAX;
}

static int da9063_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	return dm_i2c_read(dev->parent, reg, buff, len);
}

static int da9063_write(struct udevice *dev, uint reg, const u8 *buff, int len)
{
	return dm_i2c_write(dev->parent, reg, buff, len);
}

static int da9063_wdt_disable_timer(struct udevice *dev)
{
	u8 val;

	da9063_read(dev, DA9063_REG_CONTROL_D, &val, 1);
	val &= ~DA9063_TWDSCALE_MASK;
	val |= DA9063_TWDSCALE_DISABLE;
	da9063_write(dev, DA9063_REG_CONTROL_D, &val, 1);

	return 0;
}

static int da9063_wdt_update_timeout(struct udevice *dev, unsigned int timeout)
{
	unsigned int regval;
	int ret;
	u8 val;

	/*
	 * The watchdog triggers a reboot if a timeout value is already
	 * programmed because the timeout value combines two functions
	 * in one: indicating the counter limit and starting the watchdog.
	 * The watchdog must be disabled to be able to change the timeout
	 * value if the watchdog is already running. Then we can set the
	 * new timeout value which enables the watchdog again.
	 */
	ret = da9063_wdt_disable_timer(dev);
	if (ret)
		return ret;

	udelay(300);

	regval = da9063_wdt_timeout_to_sel(timeout);

	da9063_read(dev, DA9063_REG_CONTROL_D, &val, 1);
	val &= ~DA9063_TWDSCALE_MASK;
	val |= regval;
	da9063_write(dev, DA9063_REG_CONTROL_D, &val, 1);

	return 0;
}

static int da9063_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	return da9063_wdt_update_timeout(dev, timeout);
}

static int da9063_wdt_stop(struct udevice *dev)
{
	return da9063_wdt_disable_timer(dev);
}

static int da9063_wdt_reset(struct udevice *dev)
{
	u8 val = DA9063_WATCHDOG;

	return da9063_write(dev, DA9063_REG_CONTROL_F, &val, 1);
}

static int da9063_wdt_expire_now(struct udevice *dev, ulong flags)
{
	u8 val = DA9063_SHUTDOWN;

	return da9063_write(dev, DA9063_REG_CONTROL_F, &val, 1);
}

static const struct wdt_ops da9063_wdt_ops = {
	.start = da9063_wdt_start,
	.stop = da9063_wdt_stop,
	.reset = da9063_wdt_reset,
	.expire_now = da9063_wdt_expire_now,
};

static const struct udevice_id da9063_wdt_ids[] = {
	{ .compatible = "dlg,da9063-watchdog", },
	{}
};

U_BOOT_DRIVER(da9063_wdt) = {
	.name = "da9063-wdt",
	.id = UCLASS_WDT,
	.of_match = da9063_wdt_ids,
	.ops = &da9063_wdt_ops,
};
