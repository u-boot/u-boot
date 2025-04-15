// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Converted to driver model by Nathan Barrett-Morrison
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 *
 * adi_wtd.c - driver for ADI on-chip watchdog
 *
 */

#include <clk.h>
#include <dm.h>
#include <wdt.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/io.h>

#define WDOG_CTL  0x0
#define WDOG_CNT  0x4
#define WDOG_STAT 0x8

#define RCU_CTL   0x0
#define RCU_STAT  0x4

#define SEC_GCTL  0x0
#define SEC_FCTL  0x10
#define SEC_SCTL0 0x800

#define WDEN      0x0010
#define WDDIS     0x0AD0

struct adi_wdt_priv {
	void __iomem *rcu_base;
	void __iomem *sec_base;
	void __iomem *wdt_base;
	struct clk clock;
};

static int adi_wdt_reset(struct udevice *dev)
{
	struct adi_wdt_priv *priv = dev_get_priv(dev);

	iowrite32(0, priv->wdt_base + WDOG_STAT);

	return 0;
}

static int adi_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct adi_wdt_priv *priv = dev_get_priv(dev);

	/* Disable SYSCD_RESETb input and clear the RCU0 reset status */
	iowrite32(0xf, priv->rcu_base + RCU_STAT);
	iowrite32(0x0, priv->rcu_base + RCU_CTL);

	/* reset the SEC controller */
	iowrite32(0x2, priv->sec_base + SEC_GCTL);
	iowrite32(0x2, priv->sec_base + SEC_FCTL);

	udelay(50);

	/* enable SEC fault event */
	iowrite32(0x1, priv->sec_base + SEC_GCTL);

	/* ANOMALY 36100004 Spurious External Fault event occurs when FCTL
	 * is re-programmed when currently active fault is not cleared
	 */
	iowrite32(0xc0, priv->sec_base + SEC_FCTL);
	iowrite32(0xc1, priv->sec_base + SEC_FCTL);

	/* enable SEC fault source for watchdog0 */
	setbits_32(priv->sec_base + SEC_SCTL0 + (3*8), 0x6);

	/* Enable SYSCD_RESETb input */
	iowrite32(0x100, priv->rcu_base + RCU_CTL);

	/* enable watchdog0 */
	iowrite32(WDDIS, priv->wdt_base + WDOG_CTL);

	iowrite32(timeout_ms / 1000 *
	       (clk_get_rate(&priv->clock) / (IS_ENABLED(CONFIG_SC58X) ? 2 : 1)),
	       priv->wdt_base + WDOG_CNT);

	iowrite32(0, priv->wdt_base + WDOG_STAT);
	iowrite32(WDEN, priv->wdt_base + WDOG_CTL);

	return 0;
}

static int adi_wdt_probe(struct udevice *dev)
{
	struct adi_wdt_priv *priv = dev_get_priv(dev);
	int ret;
	struct resource res;

	ret = dev_read_resource_byname(dev, "rcu", &res);
	if (ret)
		return ret;
	priv->rcu_base = devm_ioremap(dev, res.start, resource_size(&res));

	ret = dev_read_resource_byname(dev, "sec", &res);
	if (ret)
		return ret;
	priv->sec_base = devm_ioremap(dev, res.start, resource_size(&res));

	ret = dev_read_resource_byname(dev, "wdt", &res);
	if (ret)
		return ret;
	priv->wdt_base = devm_ioremap(dev, res.start, resource_size(&res));

	ret = clk_get_by_name(dev, "sclk0", &priv->clock);
	if (ret < 0) {
		printf("Can't get WDT clk: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct wdt_ops adi_wdt_ops = {
	.start		= adi_wdt_start,
	.reset		= adi_wdt_reset,
};

static const struct udevice_id adi_wdt_ids[] = {
	{ .compatible = "adi,wdt" },
	{}
};

U_BOOT_DRIVER(adi_wdt) = {
	.name		= "adi_wdt",
	.id		= UCLASS_WDT,
	.of_match	= adi_wdt_ids,
	.probe		= adi_wdt_probe,
	.ops		= &adi_wdt_ops,
	.priv_auto = sizeof(struct adi_wdt_priv),
	.flags		= DM_FLAG_PRE_RELOC,
};
