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
#include <wdt.h>
#include <linux/io.h>

/* Refresh Register Masks */
#define XWT_WWREF_GWRR_MASK	BIT(0) /* Refresh and start new period */

/* Generic Control/Status Register Masks */
#define XWT_WWCSR_GWEN_MASK	BIT(0) /* Enable Bit */

struct wwdt_regs {
	u32 reserved0[1024];
	u32 refresh;		/* Refresh Register [0x1000] */
	u32 reserved1[1023];
	u32 csr;		/* Control/Status Register [0x2000] */
	u32 reserved2;
	u32 offset;		/* Offset Register [0x2008] */
	u32 reserved3;
	u32 cmp0;		/* Compare Value Register0 [0x2010] */
	u32 cmp1;		/* Compare Value Register1 [0x2014] */
	u32 reserved4[1006];
	u32 warmrst;		/* Warm Reset Register [0x2FD0] */
};

struct xlnx_wwdt_priv {
	bool enable_once;
	struct wwdt_regs *regs;
	struct clk clk;
};

struct xlnx_wwdt_platdata {
	bool enable_once;
	phys_addr_t iobase;
};

static int xlnx_wwdt_reset(struct udevice *dev)
{
	struct xlnx_wwdt_priv *wdt = dev_get_priv(dev);

	dev_dbg(dev, "%s ", __func__);

	writel(XWT_WWREF_GWRR_MASK, &wdt->regs->refresh);

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
	csr = readl(&wdt->regs->csr);
	csr &= ~(XWT_WWCSR_GWEN_MASK);
	writel(csr, &wdt->regs->csr);

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

	dev_dbg(dev, "%s:\n", __func__);

	clock_f = clk_get_rate(&wdt->clk);
	if (IS_ERR_VALUE(clock_f)) {
		dev_err(dev, "failed to get rate\n");
		return clock_f;
	}

	dev_dbg(dev, "%s: CLK %ld\n", __func__, clock_f);

	/* Calculate timeout count */
	count = timeout * clock_f;

	/* clk_enable will return -ENOSYS when it is not implemented */
	ret = clk_enable(&wdt->clk);
	if (ret && ret != -ENOSYS) {
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
	csr = readl(&wdt->regs->csr);
	csr &= ~(XWT_WWCSR_GWEN_MASK);
	writel(csr, &wdt->regs->csr);

	/* Set compare and offset registers for generic watchdog timeout */
	writel((u32)count, &wdt->regs->cmp0);
	writel((u32)0, &wdt->regs->cmp1);
	writel((u32)count, &wdt->regs->offset);

	/* Enable the generic watchdog timer */
	csr = readl(&wdt->regs->csr);
	csr |= (XWT_WWCSR_GWEN_MASK);
	writel(csr, &wdt->regs->csr);

	return 0;
}

static int xlnx_wwdt_probe(struct udevice *dev)
{
	int ret;
	struct xlnx_wwdt_platdata *platdata = dev_get_platdata(dev);
	struct xlnx_wwdt_priv *wdt = dev_get_priv(dev);

	dev_dbg(dev, "%s: Probing wdt%u\n", __func__, dev->seq);

	wdt->regs = (struct wwdt_regs *)ioremap(platdata->iobase,
						sizeof(struct wwdt_regs));
	wdt->enable_once = platdata->enable_once;

	ret = clk_get_by_index(dev, 0, &wdt->clk);
	if (ret < 0)
		dev_err(dev, "failed to get clock\n");

	return ret;
}

static int xlnx_wwdt_ofdata_to_platdata(struct udevice *dev)
{
	struct xlnx_wwdt_platdata *platdata = dev_get_platdata(dev);

	platdata->iobase = dev_read_addr(dev);
	platdata->enable_once = dev_read_u32_default(dev,
						     "xlnx,wdt-enable-once", 0);
	dev_dbg(dev, "wdt-enable-once %d\n", platdata->enable_once);

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
	.priv_auto_alloc_size = sizeof(struct xlnx_wwdt_priv),
	.platdata_auto_alloc_size = sizeof(struct xlnx_wwdt_platdata),
	.ofdata_to_platdata = xlnx_wwdt_ofdata_to_platdata,
	.ops = &xlnx_wwdt_ops,
};
