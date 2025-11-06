// SPDX-License-Identifier: GPL-2.0-only
/* Renesas Multi-Protocol PHY device driver
 *
 * Copyright (C) 2025 Renesas Electronics Corporation
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <generic-phy.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <reset.h>

/* Common registers */
#define MPPHY_CMNCNT1			0x80000
#define MPPHY_CMNCNT2			0x80004
#define MPPHY_PCS0REG1			0x85000
#define MPPHY_PCS0REG5			0x85010

/* Channel register base and offsets */
#define MPPHY_CHAN_BASE(ch)		(0x81000 + (ch) * 0x1000)
#define MPPHY_PXTEST_OFFSET		0x00C
#define MPPHY_RXCNT_OFFSET		0x038
#define MPPHY_SRAMCNT_OFFSET		0x040
#define MPPHY_REFCLK_OFFSET		0x014
#define MPPHY_CNTXT1_OFFSET		0x004
#define MPPHY_CNTXT2_OFFSET		0x008
#define MPPHY_TXREQ_OFFSET		0x044

/* Channel specific registers */
#define MPPHY_PXTEST(ch)		(MPPHY_CHAN_BASE(ch) + MPPHY_PXTEST_OFFSET)
#define MPPHY_PXRXCNT(ch)		(MPPHY_CHAN_BASE(ch) + MPPHY_RXCNT_OFFSET)
#define MPPHY_PXSRAMCNT(ch)		(MPPHY_CHAN_BASE(ch) + MPPHY_SRAMCNT_OFFSET)
#define MPPHY_PXREFCLK(ch)		(MPPHY_CHAN_BASE(ch) + MPPHY_REFCLK_OFFSET)
#define MPPHY_PXCNTXT1(ch)		(MPPHY_CHAN_BASE(ch) + MPPHY_CNTXT1_OFFSET)
#define MPPHY_PXCNTXT2(ch)		(MPPHY_CHAN_BASE(ch) + MPPHY_CNTXT2_OFFSET)
#define MPPHY_PXTXREQ(ch)		(MPPHY_CHAN_BASE(ch) + MPPHY_TXREQ_OFFSET)

/* Channel enable bit masks for MPPHY_CMNCNT1 register */
#define MPPHY_CMNCNT1_CH_MASK(ch)	(0xFF << ((ch) * 8))

/* Channel enable bits for MPPHY_CMNCNT1 register */
#define MPPHY_CMNCNT1_CH_EN(ch)		((ch) == 0 ? BIT(1) : BIT((ch) * 8))

/* PCS0REG5 register mask and values for each channel */
#define MPPHY_PCS0REG5_CH(ch)		(0x03 << (24 + (ch) * 2))

/* PCS0REG1 register bits */
#define MPPHY_PCS0REG1_VAL		0x00010000

/* PXTEST register bit */
#define MPPHY_PXTEST_BIT		BIT(0)

/* PXRXCNT register reset value */
#define MPPHY_PXRXCNT_RESET_VAL		0x202

/* PXSRAMCNT register bits */
#define MPPHY_PXSRAMCNT_BYPASS		BIT(0)
#define MPPHY_PXSRAMCNT_BIT3		BIT(3)
#define BOOTLOAD_BYPASS_MODE		0x3
#define SRAM_BYPASS_MODE		0xc
#define SRAM_EXT_LD_DONE		0x10
#define SRAM_INIT_DONE			0x20

#define SRAM_CONTROL_SET_BIT		\
	(BOOTLOAD_BYPASS_MODE | SRAM_BYPASS_MODE | \
	 SRAM_EXT_LD_DONE | SRAM_INIT_DONE)

/* CMNCNT1/2 clock settings */
#define MPPHY_CMNCNT2_CLK_CH(ch)	(0x30003 << ((ch) * 4))

/* PXREFCLK register value */
#define MPPHY_PXREFCLK_VAL		0x35

/* PXTXREQ register value */
#define MPPHY_PXTXREQ_VAL		0x8

/* Context settings */
#define MPPHY_CNTXT1_VALUE		0x02010002
#define MPPHY_CNTXT2_VALUE		0x02020202 /* For channels 1-3 */
#define MPPHY_CNTXT2_CH0_VALUE		0x02020201 /* Special for channel 0 */

/* struct mpphy_priv - Private data for the MPPHY driver */
struct mp_phy_priv {
	struct phy		*phy;
	struct clk_bulk		clks;
	struct reset_ctl_bulk	resets;
	void __iomem		*base;
};

static int mp_phy_init(struct phy *phy)
{
	struct mp_phy_priv *priv = dev_get_priv(phy->dev);

	if (phy->id > 3) {
		printf("Invalid channel ID: %ld\n", phy->id);
		return -EINVAL;
	}

	clrsetbits_le32(priv->base + MPPHY_CMNCNT1, MPPHY_CMNCNT1_CH_MASK(phy->id),
			MPPHY_CMNCNT1_CH_EN(phy->id));
	setbits_le32(priv->base + MPPHY_PCS0REG5, MPPHY_PCS0REG5_CH(phy->id));
	setbits_le32(priv->base + MPPHY_PCS0REG1, MPPHY_PCS0REG1_VAL);
	setbits_le32(priv->base + MPPHY_PXTEST(phy->id), MPPHY_PXTEST_BIT);
	clrbits_le32(priv->base + MPPHY_PCS0REG5, MPPHY_PCS0REG5_CH(phy->id));
	clrbits_le32(priv->base + MPPHY_PCS0REG1, MPPHY_PCS0REG1_VAL);
	clrbits_le32(priv->base + MPPHY_PXTEST(phy->id), MPPHY_PXTEST_BIT);

	/* Set PHY RX/TX reset and SRAM bypass mode */
	writel(MPPHY_PXRXCNT_RESET_VAL, priv->base + MPPHY_PXRXCNT(phy->id));
	writel(MPPHY_PXSRAMCNT_BYPASS, priv->base + MPPHY_PXSRAMCNT(phy->id));
	setbits_le32(priv->base + MPPHY_PXSRAMCNT(phy->id), MPPHY_PXSRAMCNT_BIT3);

	/* Clock supply settings */
	setbits_le32(priv->base + MPPHY_CMNCNT2, MPPHY_CMNCNT2_CLK_CH(phy->id));

	setbits_le32(priv->base + MPPHY_PXREFCLK(phy->id), MPPHY_PXREFCLK_VAL);

	/* Release PHY RX/TX reset */
	writel(0x0, priv->base + MPPHY_PXRXCNT(phy->id));

	/* Setting Context Restore Registers and select PHY2/PHY3 protocol */
	writel(MPPHY_CNTXT1_VALUE, priv->base + MPPHY_PXCNTXT1(phy->id));
	writel((phy->id == 0) ? MPPHY_CNTXT2_CH0_VALUE : MPPHY_CNTXT2_VALUE,
	       priv->base + MPPHY_PXCNTXT2(phy->id));
	writel(MPPHY_PXTXREQ_VAL, priv->base + MPPHY_PXTXREQ(phy->id));

	return 0;
}

static int mp_phy_late_init(struct phy *phy)
{
	struct mp_phy_priv *priv = dev_get_priv(phy->dev);

	writel(SRAM_CONTROL_SET_BIT, priv->base + MPPHY_PXSRAMCNT(phy->id));

	return 0;
}

static int mp_phy_set_mode(struct phy *phy, enum phy_mode mode, int submode)
{
	/* Current source code only supports Ethernet */
	if (mode != PHY_MODE_ETHERNET)
		return -EOPNOTSUPP;

	return 0;
}

static int mp_phy_of_xlate(struct phy *phy, struct ofnode_phandle_args *args)
{
	if (args->args_count > 2)
		return -EINVAL;

	/* Set channel ID from first argument if available */
	if (args->args_count)
		phy->id = args->args[0];
	else
		phy->id = 0;

	return 0;
}

static const struct phy_ops mp_phy_ops = {
	.init		= mp_phy_init,
	.power_on	= mp_phy_late_init,
	.set_mode	= mp_phy_set_mode,
	.of_xlate	= mp_phy_of_xlate,
};

static int mp_phy_probe(struct udevice *dev)
{
	struct mp_phy_priv *priv = dev_get_priv(dev);
	int ret;

	/* Get base address from device tree */
	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret < 0)
		return ret;

	ret = clk_enable_bulk(&priv->clks);
	if (ret)
		goto err_clk_enable;

	ret = reset_get_bulk(dev, &priv->resets);
	if (ret)
		goto err_reset_get;

	ret = reset_assert_bulk(&priv->resets);
	if (ret)
		goto err_reset_assert;

	ret = reset_deassert_bulk(&priv->resets);
	if (ret)
		goto err_reset_assert;

	return 0;

err_reset_assert:
	reset_release_bulk(&priv->resets);
err_reset_get:
	clk_disable_bulk(&priv->clks);
err_clk_enable:
	clk_release_bulk(&priv->clks);
	return ret;
}

static const struct udevice_id mp_phy_ids[] = {
	{ .compatible = "renesas,r8a78000-multi-protocol-phy" },
	{ }
};

U_BOOT_DRIVER(renesas_mpphy) = {
	.name		= "renesas_mpphy",
	.id		= UCLASS_PHY,
	.of_match	= mp_phy_ids,
	.probe		= mp_phy_probe,
	.ops		= &mp_phy_ops,
	.priv_auto	= sizeof(struct mp_phy_priv),
};
