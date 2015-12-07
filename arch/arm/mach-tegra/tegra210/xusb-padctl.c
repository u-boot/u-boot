/*
 * Copyright (c) 2014-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#define pr_fmt(fmt) "tegra-xusb-padctl: " fmt

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>

#include <asm/io.h>

#include <asm/arch/clock.h>
#include <asm/arch-tegra/xusb-padctl.h>

#include <dt-bindings/pinctrl/pinctrl-tegra-xusb.h>

struct tegra_xusb_phy_ops {
	int (*prepare)(struct tegra_xusb_phy *phy);
	int (*enable)(struct tegra_xusb_phy *phy);
	int (*disable)(struct tegra_xusb_phy *phy);
	int (*unprepare)(struct tegra_xusb_phy *phy);
};

struct tegra_xusb_phy {
	const struct tegra_xusb_phy_ops *ops;

	struct tegra_xusb_padctl *padctl;
};

struct tegra_xusb_padctl {
	struct fdt_resource regs;

	unsigned int enable;

	struct tegra_xusb_phy phys[2];
};

static inline u32 padctl_readl(struct tegra_xusb_padctl *padctl,
			       unsigned long offset)
{
	u32 value = readl(padctl->regs.start + offset);
	debug("padctl: %08lx > %08x\n", offset, value);
	return value;
}

static inline void padctl_writel(struct tegra_xusb_padctl *padctl,
				 u32 value, unsigned long offset)
{
	debug("padctl: %08lx < %08x\n", offset, value);
	writel(value, padctl->regs.start + offset);
}

#define XUSB_PADCTL_ELPG_PROGRAM 0x024
#define  XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_VCORE_DOWN (1 << 31)
#define  XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN_EARLY (1 << 30)
#define  XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN (1 << 29)

static int tegra_xusb_padctl_enable(struct tegra_xusb_padctl *padctl)
{
	u32 value;

	if (padctl->enable++ > 0)
		return 0;

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value &= ~XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	udelay(100);

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value &= ~XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN_EARLY;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	udelay(100);

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value &= ~XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_VCORE_DOWN;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	return 0;
}

static int tegra_xusb_padctl_disable(struct tegra_xusb_padctl *padctl)
{
	u32 value;

	if (padctl->enable == 0) {
		error("unbalanced enable/disable");
		return 0;
	}

	if (--padctl->enable > 0)
		return 0;

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value |= XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_VCORE_DOWN;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	udelay(100);

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value |= XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN_EARLY;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	udelay(100);

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value |= XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	return 0;
}

static int phy_prepare(struct tegra_xusb_phy *phy)
{
	int err;

	err = tegra_xusb_padctl_enable(phy->padctl);
	if (err < 0)
		return err;

	reset_set_enable(PERIPH_ID_PEX_USB_UPHY, 0);

	return 0;
}

static int phy_unprepare(struct tegra_xusb_phy *phy)
{
	reset_set_enable(PERIPH_ID_PEX_USB_UPHY, 1);

	return tegra_xusb_padctl_disable(phy->padctl);
}

#define XUSB_PADCTL_UPHY_PLL_P0_CTL1 0x360
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_FREQ_NDIV_MASK (0xff << 20)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_FREQ_NDIV(x) (((x) & 0xff) << 20)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_FREQ_MDIV_MASK (0x3 << 16)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_LOCKDET_STATUS (1 << 15)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_PWR_OVRD (1 << 4)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_ENABLE (1 << 3)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_SLEEP_MASK (0x3 << 1)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_SLEEP(x) (((x) & 0x3) << 1)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_IDDQ (1 << 0)

#define XUSB_PADCTL_UPHY_PLL_P0_CTL2 0x364
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_CTRL_MASK (0xffffff << 4)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_CTRL(x) (((x) & 0xffffff) << 4)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_OVRD (1 << 2)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_DONE (1 << 1)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_EN (1 << 0)

#define XUSB_PADCTL_UPHY_PLL_P0_CTL4 0x36c
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL4_TXCLKREF_EN (1 << 15)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL4_TXCLKREF_SEL_MASK (0x3 << 12)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL4_TXCLKREF_SEL(x) (((x) & 0x3) << 12)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL4_REFCLKBUF_EN (1 << 8)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL4_REFCLK_SEL_MASK (0xf << 4)

#define XUSB_PADCTL_UPHY_PLL_P0_CTL5 0x370
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL5_DCO_CTRL_MASK (0xff << 16)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL5_DCO_CTRL(x) (((x) & 0xff) << 16)

#define XUSB_PADCTL_UPHY_PLL_P0_CTL8 0x37c
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_DONE (1 << 31)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_OVRD (1 << 15)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_CLK_EN (1 << 13)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_EN (1 << 12)

#define CLK_RST_XUSBIO_PLL_CFG0 0x51c
#define  CLK_RST_XUSBIO_PLL_CFG0_SEQ_ENABLE (1 << 24)
#define  CLK_RST_XUSBIO_PLL_CFG0_PADPLL_SLEEP_IDDQ (1 << 13)
#define  CLK_RST_XUSBIO_PLL_CFG0_PADPLL_USE_LOCKDET (1 << 6)
#define  CLK_RST_XUSBIO_PLL_CFG0_CLK_ENABLE_SWCTL (1 << 2)
#define  CLK_RST_XUSBIO_PLL_CFG0_PADPLL_RESET_SWCTL (1 << 0)

static int pcie_phy_enable(struct tegra_xusb_phy *phy)
{
	struct tegra_xusb_padctl *padctl = phy->padctl;
	unsigned long start;
	u32 value;

	debug("> %s(phy=%p)\n", __func__, phy);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL2);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_CTRL_MASK;
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_CTRL(0x136);
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL2);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL5);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL5_DCO_CTRL_MASK;
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL5_DCO_CTRL(0x2a);
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL5);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL1);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL1_PWR_OVRD;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL1);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL2);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_OVRD;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL2);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL8);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_OVRD;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL8);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL4);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL4_TXCLKREF_SEL_MASK;
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL4_REFCLK_SEL_MASK;
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL4_TXCLKREF_SEL(2);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL4_TXCLKREF_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL4);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL1);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL1_FREQ_MDIV_MASK;
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL1_FREQ_NDIV_MASK;
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL1_FREQ_NDIV(25);
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL1);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL1);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL1_IDDQ;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL1);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL1);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL1_SLEEP_MASK;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL1);

	udelay(1);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL4);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL4_REFCLKBUF_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL4);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL2);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL2);

	debug("  waiting for calibration\n");

	start = get_timer(0);

	while (get_timer(start) < 250) {
		value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL2);
		if (value & XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_DONE)
			break;
	}

	debug("  done\n");

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL2);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL2);

	debug("  waiting for calibration to stop\n");

	start = get_timer(0);

	while (get_timer(start) < 250) {
		value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL2);
		if ((value & XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_DONE) == 0)
			break;
	}

	debug("  done\n");

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL1);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL1_ENABLE;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL1);

	debug("  waiting for PLL to lock...\n");
	start = get_timer(0);

	while (get_timer(start) < 250) {
		value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL1);
		if (value & XUSB_PADCTL_UPHY_PLL_P0_CTL1_LOCKDET_STATUS)
			break;
	}

	debug("  done\n");

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL8);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_CLK_EN;
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL8);

	debug("  waiting for register calibration...\n");
	start = get_timer(0);

	while (get_timer(start) < 250) {
		value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL8);
		if (value & XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_DONE)
			break;
	}

	debug("  done\n");

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL8);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL8);

	debug("  waiting for register calibration to stop...\n");
	start = get_timer(0);

	while (get_timer(start) < 250) {
		value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL8);
		if ((value & XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_DONE) == 0)
			break;
	}

	debug("  done\n");

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL8);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_CLK_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL8);

	value = readl(NV_PA_CLK_RST_BASE + CLK_RST_XUSBIO_PLL_CFG0);
	value &= ~CLK_RST_XUSBIO_PLL_CFG0_PADPLL_RESET_SWCTL;
	value &= ~CLK_RST_XUSBIO_PLL_CFG0_CLK_ENABLE_SWCTL;
	value |= CLK_RST_XUSBIO_PLL_CFG0_PADPLL_USE_LOCKDET;
	value |= CLK_RST_XUSBIO_PLL_CFG0_PADPLL_SLEEP_IDDQ;
	writel(value, NV_PA_CLK_RST_BASE + CLK_RST_XUSBIO_PLL_CFG0);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL1);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL1_PWR_OVRD;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL1);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL2);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_OVRD;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL2);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL8);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_OVRD;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL8);

	udelay(1);

	value = readl(NV_PA_CLK_RST_BASE + CLK_RST_XUSBIO_PLL_CFG0);
	value |= CLK_RST_XUSBIO_PLL_CFG0_SEQ_ENABLE;
	writel(value, NV_PA_CLK_RST_BASE + CLK_RST_XUSBIO_PLL_CFG0);

	debug("< %s()\n", __func__);
	return 0;
}

static int pcie_phy_disable(struct tegra_xusb_phy *phy)
{
	return 0;
}

static const struct tegra_xusb_phy_ops pcie_phy_ops = {
	.prepare = phy_prepare,
	.enable = pcie_phy_enable,
	.disable = pcie_phy_disable,
	.unprepare = phy_unprepare,
};

static struct tegra_xusb_padctl *padctl = &(struct tegra_xusb_padctl) {
	.phys = {
		[0] = {
			.ops = &pcie_phy_ops,
		},
	},
};

static int tegra_xusb_padctl_parse_dt(struct tegra_xusb_padctl *padctl,
				      const void *fdt, int node)
{
	int err;

	err = fdt_get_resource(fdt, node, "reg", 0, &padctl->regs);
	if (err < 0) {
		error("registers not found");
		return err;
	}

	debug("regs: %pa-%pa\n", &padctl->regs.start,
	      &padctl->regs.end);

	return 0;
}

static int process_nodes(const void *fdt, int nodes[], unsigned int count)
{
	unsigned int i;
	int err;

	debug("> %s(fdt=%p, nodes=%p, count=%u)\n", __func__, fdt, nodes,
	      count);

	for (i = 0; i < count; i++) {
		enum fdt_compat_id id;

		if (!fdtdec_get_is_enabled(fdt, nodes[i]))
			continue;

		id = fdtdec_lookup(fdt, nodes[i]);
		switch (id) {
		case COMPAT_NVIDIA_TEGRA124_XUSB_PADCTL:
		case COMPAT_NVIDIA_TEGRA210_XUSB_PADCTL:
			break;

		default:
			error("unsupported compatible: %s",
			      fdtdec_get_compatible(id));
			continue;
		}

		err = tegra_xusb_padctl_parse_dt(padctl, fdt, nodes[i]);
		if (err < 0) {
			error("failed to parse DT: %d",
			      err);
			continue;
		}

		/* deassert XUSB padctl reset */
		reset_set_enable(PERIPH_ID_XUSB_PADCTL, 0);

		/* only a single instance is supported */
		break;
	}

	debug("< %s()\n", __func__);
	return 0;
}

struct tegra_xusb_phy *tegra_xusb_phy_get(unsigned int type)
{
	struct tegra_xusb_phy *phy = NULL;

	switch (type) {
	case TEGRA_XUSB_PADCTL_PCIE:
		phy = &padctl->phys[0];
		phy->padctl = padctl;
		break;
	}

	return phy;
}

int tegra_xusb_phy_prepare(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->prepare)
		return phy->ops->prepare(phy);

	return phy ? -ENOSYS : -EINVAL;
}

int tegra_xusb_phy_enable(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->enable)
		return phy->ops->enable(phy);

	return phy ? -ENOSYS : -EINVAL;
}

int tegra_xusb_phy_disable(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->disable)
		return phy->ops->disable(phy);

	return phy ? -ENOSYS : -EINVAL;
}

int tegra_xusb_phy_unprepare(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->unprepare)
		return phy->ops->unprepare(phy);

	return phy ? -ENOSYS : -EINVAL;
}

void tegra_xusb_padctl_init(const void *fdt)
{
	int count, nodes[1];

	debug("> %s(fdt=%p)\n", __func__, fdt);

	count = fdtdec_find_aliases_for_id(fdt, "padctl",
					   COMPAT_NVIDIA_TEGRA210_XUSB_PADCTL,
					   nodes, ARRAY_SIZE(nodes));
	if (process_nodes(fdt, nodes, count))
		return;

	count = fdtdec_find_aliases_for_id(fdt, "padctl",
					   COMPAT_NVIDIA_TEGRA124_XUSB_PADCTL,
					   nodes, ARRAY_SIZE(nodes));
	if (process_nodes(fdt, nodes, count))
		return;

	debug("< %s()\n", __func__);
}
