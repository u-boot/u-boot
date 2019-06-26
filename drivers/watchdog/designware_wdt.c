// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <asm/io.h>
#include <asm/utils.h>

#define DW_WDT_CR	0x00
#define DW_WDT_TORR	0x04
#define DW_WDT_CRR	0x0C

#define DW_WDT_CR_EN_OFFSET	0x00
#define DW_WDT_CR_RMOD_OFFSET	0x01
#define DW_WDT_CR_RMOD_VAL	0x00
#define DW_WDT_CRR_RESTART_VAL	0x76

struct designware_wdt_priv {
	void __iomem	*base;
};

/*
 * Set the watchdog time interval.
 * Counter is 32 bit.
 */
static int designware_wdt_settimeout(void __iomem *base, unsigned int clk_khz,
				     unsigned int timeout)
{
	signed int i;

	/* calculate the timeout range value */
	i = log_2_n_round_up(timeout * clk_khz) - 16;
	i = clamp(i, 0, 15);

	writel(i | (i << 4), base + DW_WDT_TORR);

	return 0;
}

static void designware_wdt_enable(void __iomem *base)
{
	writel((DW_WDT_CR_RMOD_VAL << DW_WDT_CR_RMOD_OFFSET) |
		BIT(DW_WDT_CR_EN_OFFSET),
		base + DW_WDT_CR);
}

static unsigned int designware_wdt_is_enabled(void __iomem *base)
{
	return readl(base + DW_WDT_CR) & BIT(0);
}

static void designware_wdt_reset_common(void __iomem *base)
{
	if (designware_wdt_is_enabled(base))
		/* restart the watchdog counter */
		writel(DW_WDT_CRR_RESTART_VAL, base + DW_WDT_CRR);
}

#if !CONFIG_IS_ENABLED(WDT)
void hw_watchdog_reset(void)
{
	designware_wdt_reset_common((void __iomem *)CONFIG_DW_WDT_BASE);
}

void hw_watchdog_init(void)
{
	/* reset to disable the watchdog */
	hw_watchdog_reset();
	/* set timer in miliseconds */
	designware_wdt_settimeout((void __iomem *)CONFIG_DW_WDT_BASE,
				  CONFIG_DW_WDT_CLOCK_KHZ,
				  CONFIG_WATCHDOG_TIMEOUT_MSECS);
	/* enable the watchdog */
	designware_wdt_enable((void __iomem *)CONFIG_DW_WDT_BASE);
	/* reset the watchdog */
	hw_watchdog_reset();
}
#else
static int designware_wdt_reset(struct udevice *dev)
{
	struct designware_wdt_priv *priv = dev_get_priv(dev);

	designware_wdt_reset_common(priv->base);

	return 0;
}

static int designware_wdt_stop(struct udevice *dev)
{
	struct designware_wdt_priv *priv = dev_get_priv(dev);

	designware_wdt_reset(dev);
	writel(DW_WDT_CR_RMOD_VAL << DW_WDT_CR_RMOD_OFFSET,
		priv->base + DW_WDT_CR);

	return 0;
}

static int designware_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct designware_wdt_priv *priv = dev_get_priv(dev);

	designware_wdt_stop(dev);

	/* set timer in miliseconds */
	designware_wdt_settimeout(priv->base, CONFIG_DW_WDT_CLOCK_KHZ, timeout);

	designware_wdt_enable(priv->base);

	/* reset the watchdog */
	return designware_wdt_reset(dev);
}

static int designware_wdt_probe(struct udevice *dev)
{
	struct designware_wdt_priv *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;

	/* reset to disable the watchdog */
	return designware_wdt_stop(dev);
}

static const struct wdt_ops designware_wdt_ops = {
	.start = designware_wdt_start,
	.reset = designware_wdt_reset,
	.stop = designware_wdt_stop,
};

static const struct udevice_id designware_wdt_ids[] = {
	{ .compatible = "snps,dw-wdt"},
	{}
};

U_BOOT_DRIVER(designware_wdt) = {
	.name = "designware_wdt",
	.id = UCLASS_WDT,
	.of_match = designware_wdt_ids,
	.priv_auto_alloc_size = sizeof(struct designware_wdt_priv),
	.probe = designware_wdt_probe,
	.ops = &designware_wdt_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
#endif
