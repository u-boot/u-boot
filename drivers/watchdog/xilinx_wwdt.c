// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx window watchdog timer driver.
 *
 * Author(s):	Michal Simek <michal.simek@xilinx.com>
 *		Ashok Reddy Soma <ashokred@xilinx.com>
 *
 * Copyright (c) 2020, Xilinx Inc.
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <wdt.h>
#include <linux/compat.h>
#include <linux/io.h>

/* Refresh Register Masks */
#define XWT_WWREF_GWRR_MASK	BIT(0) /* Refresh and start new period */

/* Generic Control/Status Register Masks */
#define XWT_WWCSR_GWEN_MASK	BIT(0) /* Enable Bit */

/* Register offsets for the Wdt device */
#define XWT_WWREF_OFFSET	0x1000 /* Refresh Register */
#define XWT_WWCSR_OFFSET	0x2000 /* Control/Status Register */
#define XWT_WWOFF_OFFSET	0x2008 /* Offset Register */
#define XWT_WWCMP0_OFFSET	0x2010 /* Compare Value Register0 */
#define XWT_WWCMP1_OFFSET	0x2014 /* Compare Value Register1 */
#define XWT_WWWRST_OFFSET	0x2FD0 /* Warm Reset Register */

struct xlnx_wwdt_priv {
	bool enable_once;
	struct regmap *regs;
	struct clk clk;
};

struct xlnx_wwdt_plat {
	bool enable_once;
};

static int xlnx_wwdt_reset(struct udevice *dev)
{
	struct xlnx_wwdt_priv *wdt = dev_get_priv(dev);

	regmap_write(wdt->regs, XWT_WWREF_OFFSET, XWT_WWREF_GWRR_MASK);

	return 0;
}

static int xlnx_wwdt_stop(struct udevice *dev)
{
	u32 csr;
	struct xlnx_wwdt_priv *wdt = dev_get_priv(dev);

	if (wdt->enable_once) {
		dev_warn(dev, "Can't stop Xilinx watchdog.\n");
		return -EBUSY;
	}

	/* Disable the generic watchdog timer */
	regmap_read(wdt->regs, XWT_WWCSR_OFFSET, &csr);
	csr &= ~(XWT_WWCSR_GWEN_MASK);
	regmap_write(wdt->regs, XWT_WWCSR_OFFSET, csr);

	clk_disable(&wdt->clk);

	dev_dbg(dev, "Watchdog disabled!\n");

	return 0;
}

static int xlnx_wwdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	int ret;
	u32 csr;
	u64 count;
	unsigned long clock_f;
	struct xlnx_wwdt_priv *wdt = dev_get_priv(dev);

	clock_f = clk_get_rate(&wdt->clk);
	if (IS_ERR_VALUE(clock_f)) {
		dev_err(dev, "failed to get rate\n");
		return clock_f;
	}

	dev_dbg(dev, "%s: CLK %ld\n", __func__, clock_f);

	/* Calculate timeout count */
	count = timeout * clock_f;

	ret = clk_enable(&wdt->clk);
	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	/*
	 * Timeout count is half as there are two windows
	 * first window overflow is ignored (interrupt),
	 * reset is only generated at second window overflow
	 */
	count = count >> 1;

	/* Disable the generic watchdog timer */
	regmap_read(wdt->regs, XWT_WWCSR_OFFSET, &csr);
	csr &= ~(XWT_WWCSR_GWEN_MASK);
	regmap_write(wdt->regs, XWT_WWCSR_OFFSET, csr);

	/* Set compare and offset registers for generic watchdog timeout */
	regmap_write(wdt->regs, XWT_WWCMP0_OFFSET, (u32)count);
	regmap_write(wdt->regs, XWT_WWCMP1_OFFSET, 0);
	regmap_write(wdt->regs, XWT_WWOFF_OFFSET, (u32)count);

	/* Enable the generic watchdog timer */
	regmap_read(wdt->regs, XWT_WWCSR_OFFSET, &csr);
	csr |= (XWT_WWCSR_GWEN_MASK);
	regmap_write(wdt->regs, XWT_WWCSR_OFFSET, csr);

	return 0;
}

static int xlnx_wwdt_probe(struct udevice *dev)
{
	int ret;
	struct xlnx_wwdt_plat *plat = dev_get_plat(dev);
	struct xlnx_wwdt_priv *wdt = dev_get_priv(dev);

	dev_dbg(dev, "%s: Probing wdt%u\n", __func__, dev_seq(dev));

	ret = regmap_init_mem(dev_ofnode(dev), &wdt->regs);
	if (ret) {
		dev_dbg(dev, "failed to get regbase of wwdt\n");
		return ret;
	}

	wdt->enable_once = plat->enable_once;

	ret = clk_get_by_index(dev, 0, &wdt->clk);
	if (ret < 0)
		dev_err(dev, "failed to get clock\n");

	return ret;
}

static int xlnx_wwdt_of_to_plat(struct udevice *dev)
{
	struct xlnx_wwdt_plat *plat = dev_get_plat(dev);

	plat->enable_once = dev_read_u32_default(dev, "xlnx,wdt-enable-once",
						 0);
	dev_dbg(dev, "wdt-enable-once %d\n", plat->enable_once);

	return 0;
}

static const struct wdt_ops xlnx_wwdt_ops = {
	.start = xlnx_wwdt_start,
	.reset = xlnx_wwdt_reset,
	.stop = xlnx_wwdt_stop,
};

static const struct udevice_id xlnx_wwdt_ids[] = {
	{ .compatible = "xlnx,versal-wwdt-1.0", },
	{},
};

U_BOOT_DRIVER(xlnx_wwdt) = {
	.name = "xlnx_wwdt",
	.id = UCLASS_WDT,
	.of_match = xlnx_wwdt_ids,
	.probe = xlnx_wwdt_probe,
	.priv_auto	= sizeof(struct xlnx_wwdt_priv),
	.plat_auto	= sizeof(struct xlnx_wwdt_plat),
	.of_to_plat = xlnx_wwdt_of_to_plat,
	.ops = &xlnx_wwdt_ops,
};
