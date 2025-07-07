// SPDX-License-Identifier: GPL-2.0+
// Copyright 2025 Red Hat, Inc., Shmuel Leib Melamud <smelamud@redhat.com>

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <wdt.h>

#define usleep_range(a, b) udelay((a))

struct rwdt {
	u32 cnt;
	u32 csra;
	u32 csrb;
};

#define RWTCSRA_WOVF	BIT(4)
#define RWTCSRA_WRFLG	BIT(5)
#define RWTCSRA_TME		BIT(7)

#define CSR_MASK	0xA5A5A500
#define CNT_MASK	0x5A5A0000

#define MAX_CNT_VALUE   65536
/*
 * In probe, clk_rate is checked to be not more than 16 bit * biggest clock
 * divider (12 bits). d is only a factor to fully utilize the WDT counter and
 * will not exceed its 16 bits. Thus, no overflow, we stay below 32 bits.
 */
#define MUL_BY_CLKS_PER_SEC(p, d) \
	DIV_ROUND_UP((d) * (p)->clk_rate, clk_divs[(p)->cks])

/* d is 16 bit, clk_divs 12 bit -> no 32 bit overflow */
#define DIV_BY_CLKS_PER_SEC(p, d) ((d) * clk_divs[(p)->cks] / (p)->clk_rate)

static const unsigned int clk_divs[] = { 1, 4, 16, 32, 64, 128, 1024, 4096 };

struct rwdt_priv {
	struct rwdt __iomem *wdt;
	unsigned long clk_rate;
	u8 cks;
	struct clk clk;
};

static void rwdt_wait_cycles(struct rwdt_priv *priv, unsigned int cycles)
{
	unsigned int delay;

	delay = DIV_ROUND_UP(cycles * 1000000, priv->clk_rate);

	usleep_range(delay, 2 * delay);
}

static int rwdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct rwdt_priv *priv = dev_get_priv(dev);
	u64 max_timeout;
	u8 val;

	max_timeout = DIV_BY_CLKS_PER_SEC(priv, MAX_CNT_VALUE);
	timeout = min(max_timeout, timeout / 1000);

	/* Stop the timer before we modify any register */
	val = readb_relaxed(&priv->wdt->csra) & ~RWTCSRA_TME;
	writel_relaxed(val | CSR_MASK, &priv->wdt->csra);
	/* Delay 2 cycles before setting watchdog counter */
	rwdt_wait_cycles(priv, 2);

	while (readb_relaxed(&priv->wdt->csra) & RWTCSRA_WRFLG)
		cpu_relax();

	writel_relaxed((MAX_CNT_VALUE - MUL_BY_CLKS_PER_SEC(priv, timeout))
		       | CNT_MASK, &priv->wdt->cnt);

	writel_relaxed(priv->cks | RWTCSRA_TME | CSR_MASK, &priv->wdt->csra);

	return 0;
}

static int rwdt_stop(struct udevice *dev)
{
	struct rwdt_priv *priv = dev_get_priv(dev);

	writel_relaxed(priv->cks | CSR_MASK, &priv->wdt->csra);

	return 0;
}

static int rwdt_reset(struct udevice *dev)
{
	struct rwdt_priv *priv = dev_get_priv(dev);
	u8 val;

	/* Stop the timer before we modify any register */
	val = readb_relaxed(&priv->wdt->csra) & ~RWTCSRA_TME;
	writel_relaxed(val | CSR_MASK, &priv->wdt->csra);
	/* Delay 2 cycles before setting watchdog counter */
	rwdt_wait_cycles(priv, 2);

	writel_relaxed(0xffff | CNT_MASK, &priv->wdt->cnt);
	/* smallest divider to reboot soon */
	writel_relaxed(0 | CSR_MASK, &priv->wdt->csra);

	readb_poll_timeout(&priv->wdt->csra, val, !(val & RWTCSRA_WRFLG), 100);

	writel_relaxed(RWTCSRA_TME | CSR_MASK, &priv->wdt->csra);

	/* wait 2 cycles, so watchdog will trigger */
	rwdt_wait_cycles(priv, 2);

	return 0;
}

static int rwdt_probe(struct udevice *dev)
{
	struct rwdt_priv *priv = dev_get_priv(dev);
	unsigned long clks_per_sec;
	int ret, i;

	priv->wdt = dev_remap_addr(dev);
	if (!priv->wdt)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret)
		return ret;

	priv->clk_rate = clk_get_rate(&priv->clk);
	if (!priv->clk_rate) {
		ret = -ENOENT;
		goto err_clk_disable;
	}

	/*
	 * Find the largest possible divider that allows clock rate
	 * (clks_per_sec) to stay within 16 bits. In this case, we can still
	 * measure the smallest timeout (1s) and make the largest allowed
	 * timeout as large as possible.
	 */
	for (i = ARRAY_SIZE(clk_divs) - 1; i >= 0; i--) {
		clks_per_sec = priv->clk_rate / clk_divs[i];
		if (clks_per_sec && clks_per_sec < 65536) {
			priv->cks = i;
			break;
		}
	}

	/* can't find a suitable clock divider */
	if (i < 0) {
		ret = -ERANGE;
		goto err_clk_disable;
	}

	return 0;

err_clk_disable:
	clk_disable(&priv->clk);

	return ret;
}

static const struct wdt_ops rwdt_ops = {
	.start = rwdt_start,
	.reset = rwdt_reset,
	.stop = rwdt_stop,
};

static const struct udevice_id rwdt_ids[] = {
	{ .compatible = "renesas,rcar-gen2-wdt" },
	{ .compatible = "renesas,rcar-gen3-wdt" },
	{ .compatible = "renesas,rcar-gen4-wdt" },
	{}
};

U_BOOT_DRIVER(wdt_renesas) = {
	.name = "wdt_renesas",
	.id = UCLASS_WDT,
	.of_match = rwdt_ids,
	.ops = &rwdt_ops,
	.probe	= rwdt_probe,
	.priv_auto = sizeof(struct rwdt_priv),
};
