// SPDX-License-Identifier: GPL-2.0
/*
 * NVIDIA Tegra Watchdog driver
 *
 * Copyright (C) 2025 NABLA Software Engineering
 * Lukasz Majewski, NABLA Software Engineering, lukma@nabladev.com
 */

#include <dm.h>
#include <wdt.h>
#include <hang.h>
#include <asm/io.h>
#include <watchdog.h>

/* Timer registers */
#define TIMER_PTV			0x0
#define TIMER_EN			BIT(31)
#define TIMER_PERIODIC			BIT(30)

/* WDT registers */
#define WDT_CFG				0x0
#define WDT_CFG_PERIOD_SHIFT		4
#define WDT_CFG_PERIOD_MASK		GENMASK(7, 0)
#define WDT_CFG_INT_EN			BIT(12)
#define WDT_CFG_PMC2CAR_RST_EN		BIT(15)
#define WDT_CMD				0x8
#define WDT_CMD_START_COUNTER		BIT(0)
#define WDT_CMD_DISABLE_COUNTER		BIT(1)
#define WDT_UNLOCK			0xc
#define WDT_UNLOCK_PATTERN		0xc45a

/* Use watchdog ID 0 */
#define WDT_BASE			0x100

/* Use Timer 5 as WDT counter */
#define WDT_TIMER_BASE			0x60
#define WDT_TIMER_ID			5

struct tegra_wdt_priv {
	void __iomem *wdt_base;
	void __iomem *tmr_base;
};

static int tegra_wdt_reset(struct udevice *dev)
{
	struct tegra_wdt_priv *priv = dev_get_priv(dev);

	writel(WDT_CMD_START_COUNTER, priv->wdt_base + WDT_CMD);

	return 0;
}

static int tegra_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct tegra_wdt_priv *priv = dev_get_priv(dev);
	u32 timeout_sec = timeout / 1000;

	/* Support for timeout from 1 to 255 seconds */
	if (timeout_sec < 1 || timeout_sec > 255)
		return -EINVAL;

	/*
	 * Timer for WDT has a fixed 1MHz clock, so for 1 second period one
	 * shall write 1000000ul.
	 *
	 * On Tegra the watchdog reset actually occurs on the 4th expiration
	 * of this counter, so we set the period to 1/4.
	 */
	writel(TIMER_EN | TIMER_PERIODIC | (1000000ul / 4),
	       priv->tmr_base + TIMER_PTV);

	writel(WDT_CFG_PMC2CAR_RST_EN | (timeout_sec << WDT_CFG_PERIOD_SHIFT) |
	       WDT_TIMER_ID, priv->wdt_base + WDT_CFG);

	writel(WDT_CMD_START_COUNTER, priv->wdt_base + WDT_CMD);

	return 0;
}

static int tegra_wdt_stop(struct udevice *dev)
{
	struct tegra_wdt_priv *priv = dev_get_priv(dev);

	writel(WDT_UNLOCK_PATTERN, priv->wdt_base + WDT_UNLOCK);
	writel(WDT_CMD_DISABLE_COUNTER, priv->wdt_base + WDT_CMD);
	writel(0, priv->tmr_base + TIMER_PTV);

	return 0;
}

static int tegra_wdt_probe(struct udevice *dev)
{
	struct tegra_wdt_priv *priv = dev_get_priv(dev);
	void __iomem *base;

	base = dev_read_addr_ptr(dev);
	if (!base)
		return -ENOENT;

	priv->wdt_base = base + WDT_BASE;
	priv->tmr_base = base + WDT_TIMER_BASE;

	return 0;
}

static const struct wdt_ops tegra_wdt_ops = {
	.start		= tegra_wdt_start,
	.stop		= tegra_wdt_stop,
	.reset		= tegra_wdt_reset,
};

U_BOOT_DRIVER(tegra_wdt) = {
	.name = "tegra_wdt",
	.id = UCLASS_WDT,
	.probe = tegra_wdt_probe,
	.ops = &tegra_wdt_ops,
	.priv_auto = sizeof(struct tegra_wdt_priv),
};
