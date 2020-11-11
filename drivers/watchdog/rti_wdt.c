// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) Siemens AG, 2020
 *
 * Authors:
 *   Jan Kiszka <jan.kiszka@siemens.com>
 *
 * Derived from linux/drivers/watchdog/rti_wdt.c
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <power-domain.h>
#include <wdt.h>
#include <asm/io.h>

/* Timer register set definition */
#define RTIDWDCTRL		0x90
#define RTIDWDPRLD		0x94
#define RTIWDSTATUS		0x98
#define RTIWDKEY		0x9c
#define RTIDWDCNTR		0xa0
#define RTIWWDRXCTRL		0xa4
#define RTIWWDSIZECTRL		0xa8

#define RTIWWDRX_NMI		0xa

#define RTIWWDSIZE_50P		0x50

#define WDENABLE_KEY		0xa98559da

#define WDKEY_SEQ0		0xe51a
#define WDKEY_SEQ1		0xa35c

#define WDT_PRELOAD_SHIFT	13

#define WDT_PRELOAD_MAX		0xfff

struct rti_wdt_priv {
	phys_addr_t regs;
	unsigned int clk_khz;
};

static int rti_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct rti_wdt_priv *priv = dev_get_priv(dev);
	u32 timer_margin;
	int ret;

	if (readl(priv->regs + RTIDWDCTRL) == WDENABLE_KEY)
		return -EBUSY;

	timer_margin = timeout_ms * priv->clk_khz / 1000;
	timer_margin >>= WDT_PRELOAD_SHIFT;
	if (timer_margin > WDT_PRELOAD_MAX)
		timer_margin = WDT_PRELOAD_MAX;

	writel(timer_margin, priv->regs + RTIDWDPRLD);
	writel(RTIWWDRX_NMI, priv->regs + RTIWWDRXCTRL);
	writel(RTIWWDSIZE_50P, priv->regs + RTIWWDSIZECTRL);

	readl(priv->regs + RTIWWDSIZECTRL);

	writel(WDENABLE_KEY, priv->regs + RTIDWDCTRL);

	return 0;
}

static int rti_wdt_reset(struct udevice *dev)
{
	struct rti_wdt_priv *priv = dev_get_priv(dev);
	u32 prld;

	/* Make sure we do not reset too early */
	prld = readl(priv->regs + RTIDWDPRLD) << WDT_PRELOAD_SHIFT;
	if (readl(priv->regs + RTIDWDCNTR) >= prld / 2)
		return -EPERM;

	writel(WDKEY_SEQ0, priv->regs + RTIWDKEY);
	writel(WDKEY_SEQ1, priv->regs + RTIWDKEY);

	return 0;
}

static int rti_wdt_probe(struct udevice *dev)
{
	struct rti_wdt_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;

	priv->regs = devfdt_get_addr(dev);
	if (!priv->regs)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	priv->clk_khz = clk_get_rate(&clk);

	return 0;
}

static const struct wdt_ops rti_wdt_ops = {
	.start = rti_wdt_start,
	.reset = rti_wdt_reset,
};

static const struct udevice_id rti_wdt_ids[] = {
	{ .compatible = "ti,j7-rti-wdt" },
	{ }
};

U_BOOT_DRIVER(rti_wdt) = {
	.name = "rti_wdt",
	.id = UCLASS_WDT,
	.of_match = rti_wdt_ids,
	.ops = &rti_wdt_ops,
	.probe = rti_wdt_probe,
	.priv_auto_alloc_size = sizeof(struct rti_wdt_priv),
	.flags = DM_FLAG_REMOVE_WITH_PD_ON,
};
