// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Copyright (c) Linaro Ltd. 2024
 *
 * Authors:
 *   Casey Connolly <casey.connolly@linaro.org>
 *   Paul Sajna <hello@paulsajna.com>
 *
 * Derived from linux/drivers/watchdog/qcom-wdt.c
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <wdt.h>
#include <clk.h>

#include <asm/io.h>

/* Maximum allowed timeout value in Qcom SoCs*/
#define QCOM_WDT_MAX_TIMEOUT	0xfffff

enum wdt_reg {
	WDT_RST,
	WDT_EN,
	WDT_STS,
	WDT_BARK_TIME,
	WDT_BITE_TIME,
};

struct qcom_wdt_match_data {
	const u32 *offset;
};

struct qcom_wdt {
	void __iomem *base;
	ulong clk_rate;
	const u32 *layout;
};

static const u32 reg_offset_data_kpss[] = {
	[WDT_RST] = 0x4,
	[WDT_EN] = 0x8,
	[WDT_STS] = 0xC,
	[WDT_BARK_TIME] = 0x10,
	[WDT_BITE_TIME] = 0x14,
};

static const struct qcom_wdt_match_data match_data_kpss = {
	.offset = reg_offset_data_kpss,
};

static void __iomem *wdt_addr(struct qcom_wdt *wdt, enum wdt_reg reg)
{
	return wdt->base + wdt->layout[reg];
}

int qcom_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct qcom_wdt *wdt = dev_get_priv(dev);
	u64 tmp_timeout;
	u32 bark_timeout_s, bite_timeout_s;

	/* Compute timeout in watchdog ticks */
	tmp_timeout = (timeout_ms * (u64)wdt->clk_rate) / 1000;
	if (tmp_timeout > QCOM_WDT_MAX_TIMEOUT) {
		dev_warn(dev,
			 "Requested timeout (%llu ms) exceeds maximum allowed value (%llu ms). "
			 "Using max timeout instead.\n",
			 timeout_ms,
			 ((u64)QCOM_WDT_MAX_TIMEOUT * 1000) / wdt->clk_rate);
		tmp_timeout = (u32)QCOM_WDT_MAX_TIMEOUT;
		timeout_ms = (tmp_timeout * 1000) / wdt->clk_rate;
	}

	bite_timeout_s = (u32)tmp_timeout;
	tmp_timeout = ((timeout_ms - 1) * (u64)wdt->clk_rate) / 1000;
	bark_timeout_s = (u32)tmp_timeout;

	writel(0, wdt_addr(wdt, WDT_EN));
	writel(BIT(0), wdt_addr(wdt, WDT_RST));
	writel(bark_timeout_s, wdt_addr(wdt, WDT_BARK_TIME));
	writel(bite_timeout_s, wdt_addr(wdt, WDT_BITE_TIME));
	writel(BIT(0), wdt_addr(wdt, WDT_EN));

	return 0;
}

int qcom_wdt_stop(struct udevice *dev)
{
	struct qcom_wdt *wdt = dev_get_priv(dev);

	writel(0, wdt_addr(wdt, WDT_EN));
	if (readl(wdt_addr(wdt, WDT_EN))) {
		dev_err(dev, "Failed to disable Qualcomm watchdog!\n");
		return -EIO;
	}

	return 0;
}

int qcom_wdt_reset(struct udevice *dev)
{
	struct qcom_wdt *wdt = dev_get_priv(dev);

	writel(1, wdt_addr(wdt, WDT_RST));
	return 0;
}

static int qcom_wdt_probe(struct udevice *dev)
{
	struct clk clk;
	long rate;
	int ret;

	struct qcom_wdt *wdt = dev_get_priv(dev);
	struct qcom_wdt_match_data *data = (void *)dev_get_driver_data(dev);

	wdt->base = dev_read_addr_ptr(dev);
	wdt->layout = data->offset;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	rate = clk_get_rate(&clk);
	if (rate <= 0)
		return rate < 0 ? (int)rate : -EINVAL;

	wdt->clk_rate = (ulong)rate;

	return 0;
}

static const struct wdt_ops qcom_wdt_ops = {
	.start = qcom_wdt_start,
	.stop = qcom_wdt_stop,
	.reset = qcom_wdt_reset,
};

static const struct udevice_id qcom_wdt_ids[] = {
	{ .compatible = "qcom,kpss-wdt", .data = (ulong)&match_data_kpss },
	{}
};

U_BOOT_DRIVER(qcom_wdt) = {
	.name = "qcom_wdt",
	.id = UCLASS_WDT,
	.of_match = qcom_wdt_ids,
	.ops = &qcom_wdt_ops,
	.probe = qcom_wdt_probe,
	.priv_auto = sizeof(struct qcom_wdt),
};
