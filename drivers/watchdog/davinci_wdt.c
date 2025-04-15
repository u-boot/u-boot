// SPDX-License-Identifier: GPL-2.0+
/*
 * DaVinci Watchdog driver
 *
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <wdt.h>

/* Control Register */
#define DAVINCI_WDT_ID			0x00
#define DAVINCI_WDT_TIM12		0x10
#define DAVINCI_WDT_TIM34		0x14
#define DAVINCI_WDT_PRD12		0x18
#define DAVINCI_WDT_PRD34		0x1C
#define DAVINCI_WDT_TCR			0x20
#define DAVINCI_WDT_TGCR		0x24
#define DAVINCI_WDT_WDTCR		0x28

#define DAVINCI_TCR_CONT_EN		BIT(7)

#define DAVINCI_TGCR_PLUSEN		BIT(4)
#define DAVINCI_TGCR_WDT_MODE		BIT(3)
#define DAVINCI_TGCR_TIM34RS		BIT(1)
#define DAVINCI_TGCR_TIM12RS		BIT(0)

#define DAVINCI_WDTCR_INVALID_KEY	(0x5555 << 16)
#define DAVINCI_WDTCR_WDKEY0		(0xA5C6 << 16)
#define DAVINCI_WDTCR_WDKEY1		(0xDA7E << 16)
#define DAVINCI_WDTCR_WDFLAG		BIT(15)
#define DAVINCI_WDTCR_WDEN		BIT(14)

#define DEFAULT_THRESHOLD		0xA03200000

struct davinci_wdt_priv {
	void __iomem *base;
	struct clk *ref_clk;
};

static int davinci_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct davinci_wdt_priv *priv = dev_get_priv(dev);
	ulong rate = clk_get_rate(priv->ref_clk);
	u64 threshold;

	if (!rate)
		threshold = DEFAULT_THRESHOLD;
	else
		threshold = rate * timeout_ms / 1000;

	/* Reset control registers */
	writel(0, priv->base + DAVINCI_WDT_TCR);
	writel(0, priv->base + DAVINCI_WDT_TGCR);

	/* Enable watchdog mode and timers */
	writel(DAVINCI_TGCR_WDT_MODE | DAVINCI_TGCR_TIM12RS | DAVINCI_TGCR_TIM34RS,
	       priv->base + DAVINCI_WDT_TGCR);

	/* Reset counters */
	writel(0, priv->base + DAVINCI_WDT_TIM12);
	writel(0, priv->base + DAVINCI_WDT_TIM34);

	/* Set timeout threshold */
	writel(threshold & 0xFFFFFFFF, priv->base + DAVINCI_WDT_PRD12);
	writel(threshold >> 32, priv->base + DAVINCI_WDT_PRD34);

	/* Enable counter */
	writel(DAVINCI_TCR_CONT_EN, priv->base + DAVINCI_WDT_TCR);

	/* Go to watchdog's active state */
	writel(DAVINCI_WDTCR_WDEN | DAVINCI_WDTCR_WDKEY0, priv->base + DAVINCI_WDT_WDTCR);
	writel(DAVINCI_WDTCR_WDEN | DAVINCI_WDTCR_WDKEY1, priv->base + DAVINCI_WDT_WDTCR);

	return 0;
}

static int davinci_wdt_expire_now(struct udevice *dev, ulong flags)
{
	struct davinci_wdt_priv *priv = dev_get_priv(dev);

	writel(DAVINCI_WDTCR_INVALID_KEY, priv->base + DAVINCI_WDT_WDTCR);

	return 0;
}

static int davinci_wdt_restart(struct udevice *dev)
{
	struct davinci_wdt_priv *priv = dev_get_priv(dev);

	writel(DAVINCI_WDTCR_WDEN | DAVINCI_WDTCR_WDKEY0, priv->base + DAVINCI_WDT_WDTCR);
	writel(DAVINCI_WDTCR_WDEN | DAVINCI_WDTCR_WDKEY1, priv->base + DAVINCI_WDT_WDTCR);

	return 0;
}

static int davinci_wdt_probe(struct udevice *dev)
{
	struct davinci_wdt_priv *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr_index(dev, 0);
	if (!priv->base)
		return -EFAULT;

	priv->ref_clk = devm_clk_get(dev, "ref");
	if (IS_ERR(priv->ref_clk))
		return PTR_ERR(priv->ref_clk);

	return 0;
}

static const struct wdt_ops davinci_wdt_ops = {
	.start = davinci_wdt_start,
	.reset = davinci_wdt_restart,
	.expire_now = davinci_wdt_expire_now,
};

static const struct udevice_id davinci_wdt_ids[] = {
	{.compatible = "ti,davinci-wdt"},
	{}
};

U_BOOT_DRIVER(davinci_wdt) = {
	.name = "davinci_wdt",
	.id = UCLASS_WDT,
	.probe = davinci_wdt_probe,
	.of_match = davinci_wdt_ids,
	.ops = &davinci_wdt_ops,
	.priv_auto = sizeof(struct davinci_wdt_priv),
};
