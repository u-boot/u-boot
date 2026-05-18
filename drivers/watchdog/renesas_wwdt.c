// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Marek Vasut <marek.vasut+renesas@mailbox.org>
 */

#include <asm/io.h>
#include <clk.h>
#include <dm/device_compat.h>
#include <dm.h>
#include <linux/bitfield.h>
#include <linux/iopoll.h>
#include <regmap.h>
#include <syscon.h>
#include <wdt.h>

#define field_prep(_mask, _val) (((_val) << (ffs(_mask) - 1)) & (_mask))

#define RSIP_CTL_CFG4		0xc0
#define RSIP_CTL_CFG4_OPWDEN	BIT(3)
#define RSIP_CTL_CFG4_OPWDVAC	BIT(5)

#define WDTA0_ACT_CODE		0xac
#define WDTA0WDTE		0x0
#define WDTA0EVAC		0x4
#define WDTA0REF		0x8
#define WDTA0MD			0xc
#define WDTA0MD_OVF_MASK	GENMASK(6, 4)
#define WDTA0MD_OVF(n)		field_prep(WDTA0MD_OVF_MASK, (n))
#define WDTA0MD_NWIE		BIT(3)
#define WDTA0MD_NERM		BIT(2)
#define WDTA0MD_NVS_MASK	GENMASK(1, 0)
#define WDTA0MD_NVS_75P		FIELD_PREP(WDTA0MD_NVS_MASK, 3)

struct wwdt_priv {
	void __iomem *base;
	struct regmap *ctl;
	unsigned int timeout;
};

/**
 * wwdt_reset() - Reset or ping Window WDT
 * @dev: Watchdog device
 */
static int wwdt_reset(struct udevice *dev)
{
	struct wwdt_priv *priv = dev_get_priv(dev);
	const u32 cfg = readl(priv->ctl->ranges[0].start + RSIP_CTL_CFG4);
	u32 rv;

	/* WDT disabled, do nothing. */
	if (!(cfg & RSIP_CTL_CFG4_OPWDEN))
		return 0;

	/* WDT with variable activation code */
	if (cfg & RSIP_CTL_CFG4_OPWDVAC) {
		rv = readb(priv->base + WDTA0REF);
		rv = WDTA0_ACT_CODE - rv;
		writeb(rv, priv->base + WDTA0EVAC);
	} else {
		writeb(WDTA0_ACT_CODE, priv->base + WDTA0WDTE);
	}

	return 0;
}

/**
 * wwdt_start() - Start Window WDT
 * @dev: Watchdog device
 * @timeout: Watchdog timeout (not used)
 * @flags: Flags (not used)
 */
static int wwdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct wwdt_priv *priv = dev_get_priv(dev);

	clrsetbits_8(priv->base + WDTA0MD,
		     WDTA0MD_OVF_MASK | WDTA0MD_NWIE |
		     WDTA0MD_NERM | WDTA0MD_NVS_MASK,
		     WDTA0MD_OVF(priv->timeout) | WDTA0MD_NWIE |
		     WDTA0MD_NVS_75P);

	wwdt_reset(dev);

	return 0;
}

/**
 * wwdt_probe() - Initialize Window WDT hardware
 * @dev: Watchdog device
 */
static int wwdt_probe(struct udevice *dev)
{
	struct wwdt_priv *priv = dev_get_priv(dev);
	struct udevice *syscon;
	unsigned long rate;
	struct clk *clk;
	int ret;

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = uclass_get_device_by_phandle(UCLASS_SYSCON, dev, "syscon", &syscon);
	if (ret) {
		dev_err(dev, "Failed to get syscon\n");
		return ret;
	}

	priv->ctl = syscon_get_regmap(syscon);
	if (!priv->ctl) {
		dev_err(dev, "Failed to get regmap\n");
		return -ENODEV;
	}

	clk = devm_clk_get(dev, "cnt");
	if (IS_ERR(clk)) {
		ret = PTR_ERR(clk);
		dev_err(dev, "Failed to get counter clock: %d\n", ret);
		return ret;
	}

	ret = clk_enable(clk);
	if (ret)
		return ret;

	rate = clk_get_rate(clk);
	if (!rate) {
		clk_disable(clk);
		return -ENOENT;
	}

	/*
	 * Interval time is in "2^9..2^16 / clk_wdt" range. WDTA0OVFx is
	 * in 0..7 range. The code below does the WDTA0OVFx calculation
	 * from "interval_time = (1 << N) / clk_wdt" by caculating the N.
	 * The N rounded down is MSbit of (interval_time * clk_wdt). The
	 * result is then clamped to fit into the N in 9..16 range, and
	 * decremented by 9 to fit into WDTA0OVFx in 0..7 range .
	 */
	priv->timeout = clamp(fls(CONFIG_WATCHDOG_TIMEOUT_MSECS * rate) - 1, 9, 16) - 9;

	return 0;
}

static const struct wdt_ops wwdt_ops = {
	.start = wwdt_start,
	.reset = wwdt_reset,
};

static const struct udevice_id wwdt_ids[] = {
	{ .compatible = "renesas,rcar-gen5-wwdt" },
	{}
};

U_BOOT_DRIVER(wwdt_renesas) = {
	.name = "wwdt_renesas",
	.id = UCLASS_WDT,
	.of_match = wwdt_ids,
	.ops = &wwdt_ops,
	.probe	= wwdt_probe,
	.priv_auto = sizeof(struct wwdt_priv),
};
